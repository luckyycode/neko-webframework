//
//          *                  *
//             __                *
//           ,db'    *     *
//          ,d8/       *        *    *
//          888
//          `db\       *     *
//            `o`_                    **
//               *                 / )
//             *    /\__/\ *       ( (  *
//           ,-.,-.,)    (.,-.,-.,-.) ).,-.,-.
//          | @|  ={      }= | @|  / / | @|o |
//         _j__j__j_)     `-------/ /__j__j__j_
//          ________(               /___________
//          |  | @| \              || o|O | @|
//          |o |  |,'\       ,   ,'"|  |  |  |  hjw
//          vV\|/vV|`-'\  ,---\   | \Vv\hjwVv\//v
//                     _) )    `. \ /
//                    (__/       ) )
//  _   _      _           _____                                            _
// | \ | | ___| | _____   |  ___| __ __ _ _ __ ___   _____      _____  _ __| | __
// |  \| |/ _ \ |/ / _ \  | |_ | '__/ _` | '_ ` _ \ / _ \ \ /\ / / _ \| '__| |/ /
// | |\  |  __/   < (_) | |  _|| | | (_| | | | | | |  __/\ V  V / (_) | |  |   <
// |_| \_|\___|_|\_\___/  |_|  |_|  \__,_|_| |_| |_|\___| \_/\_/ \___/|_|  |_|\_\
//
//  Server.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Engine/Core/Log.h"
#include "Engine/Core/Debug.h"
#include "Engine/Core/JobSystem.h"
#include "Engine/Core/Profiler.h"
#include "Engine/Mt/Task.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Platform/SocketList.h"
#include "Engine/Network/SocketQueue.h"
#include "Engine/FS/PlatformFile.h"

#include "../Sockets/SocketSSL.h"
#include "../Sockets/SocketDefault.h"

#include "Server.h"
#include "Http.h"
#include "IProtocol.h"

#include <signal.h> // commands

namespace Neko
{
    namespace Skylar
    {
        static bool NegotiateProtocol(void* session, String& protocol)
        {
#   if USE_OPENSSL
            const Byte* proto = nullptr;
            uint32 length = 0;
            
            // ALPN
            SSL_get0_alpn_selected((SSL* )session, &proto, &length);
            if (length == 0)
            {
                // NPN
                SSL_get0_next_proto_negotiated((SSL* )session, &proto, &length);
            }
            
            bool ok = length > 0;
            if (ok)
            {
                protocol.Assign((const char*)proto, length);
            }
            
            return ok;
#   endif
        }
        
#   if USE_OPENSSL
        /**
         * Called by OpenSSL when a client sends an ALPN request to check if protocol is supported.
         * The server later will check a type and decide which protocol to use.
         */
        static int AlpnCallback(SSL* session, const Byte** out, Byte* outLength, const Byte* in, uint32 inLength, void* arg)
        {
            for (uint32 i = 0; i < inLength; i += in[i] + 1)
            {
                String protocol = String((const char* ) &in[i + 1]).Mid(0, in[i]);
                
                if (StartsWith(*protocol, "h2"))
                {
                    *out = (Byte* ) in + i + 1;
                    *outLength = in[i];
                    
                    // success
                    return SSL_TLSEXT_ERR_OK;
                }
            }
            
            // noop, fallback to http/1.1
            return SSL_TLSEXT_ERR_NOACK;
        }
#   endif
        
        // transient data
        struct SocketServerData
        {
            Server* server = nullptr;
            Net::SocketsQueue* queue = nullptr;
        };
 
        Server::Server(IAllocator& allocator, FS::IFileSystem& fileSystem)
        : Mutex(false)
        , QueueNotFullEvent(true)
        , Allocator(allocator)
        , Modules(allocator)
        , Listeners(allocator)
        , TlsData(allocator)
        , SocketsList(allocator)
        , FileSystem(fileSystem)
        , Settings(fileSystem, allocator)
        {
            SetThreadDefaultAllocator(allocator);
        };

        bool Server::Init()
        {
            LogInfo.log("Skylar") << "Server initializing..";
            
            LogInfo.log("Skylar") << "Loading server settings";
            
            // load every application & settings
            bool settingsLoaded = Settings.LoadAppSettings("serversettings.json", Modules);
            
            if (not settingsLoaded)
            {
                return false;
            }
        
#   if USE_OPENSSL
            SocketSSL::Init();
#   endif

            LogInfo.log("Skylar") << "Server initialization complete.";
            
            return true;
        }
        
        void Server::Shutdown()
        {
            LogInfo.log("Skylar") << "Server shutting down...";
            
            Stop();
            Clear();
        }

        void* Server::InitSsl(const ApplicationSettings& application)
        {
#   if USE_OPENSSL
            SSL_CTX* context = nullptr;
            
            const auto& certificate = application.CertificateFile;
            const auto& privateKey = application.KeyFile;
            
            LogInfo.log("Skylar") << "Configuring ssl configuration for the application..";
            
            context = SSL_CTX_new(SSLv23_server_method());
            
            if (SSL_CTX_use_certificate_file(context, *certificate, SSL_FILETYPE_PEM) <= 0)
            {
                LogError.log("Skylar") << "Couldn't load SSL certificate..  ";
                goto cleanupSsl;
            }
            
            if (SSL_CTX_use_PrivateKey_file(context, privateKey.IsEmpty() ? *certificate : *privateKey, SSL_FILETYPE_PEM) <= 0)
            {
                LogError.log("Skylar") << "Couldn't load SSL private key (or certificate pair)..";
                goto cleanupSsl;
            }
            
            if (not SSL_CTX_check_private_key(context))
            {
                LogError.log("Skylar") << "Couldn't verify SSL private key!";
                goto cleanupSsl;
            }
            
            // SSL_CTX_set_alpn_select_cb(context, AlpnCallback, nullptr);
            
            return context;
            
        cleanupSsl:
            {
                SSL_CTX_free(context);
                return nullptr;
            }
#   endif
        }
        
        void Server::PrepareApplications()
        {
            LogInfo.log("Skylar") << "Preparing server applications..";
            
            // Applications settings list
            TArray< ApplicationSettings* > applications(Allocator);
            
            // Get full applications settings list
            Settings.GetAllApplicationSettings(applications);
            
            // Bound port list
            TArray<uint16> ports(Allocator);
         
            // Open applications sockets
            for (auto& application : applications)
            {
                const uint16& tlsPort = application->TlsPort;
                // init ssl data for this port
                if (tlsPort != 0)
                {
                    // initialize it first
                    auto* context = InitSsl(*application);
                    
                    if (context != nullptr)
                    {
                        if (BindPort(tlsPort, ports))
                        {
                            // save context
                            TlsData.Insert(tlsPort, context);
                        }
                    }
                }
                
                BindPort(application->Port, ports);
                
                LogInfo.log("Skylar") << "Content directory: " << *application->RootDirectory;
            }
        }

        uint16 Server::Run()
        {
            bool initialized = Init();
            if (not initialized)
            {
                return EXIT_FAILURE;
            }
            
            PrepareApplications();
            
            if (Listeners.IsEmpty())
            {
                // aesthetics
                LogError.log("Skylar") << "## (⌒_⌒;) Couldn't run Skylar, no sockets were opened.";
                
                Clear();
                
                return EXIT_FAILURE;
            }
            
            // Init sockets
            SocketsList.Create(Listeners.GetSize());
            
            // Init list with created sockets
            for (const auto& socket : Listeners)
            {
                SocketsList.AddSocket(socket);
            }
            
            // start processing immediately
            Controls.SetActiveFlag();
            
            LogInfo.log("Skylar") << "Creating the main request task..";
            
            Net::SocketsQueue sockets(Allocator);
            
            // Create a job which will process all worker threads
            SocketServerData  data { this, &sockets };
            
            JobSystem::JobData job;
            job.data = (void* )&data;
            job.task = [](void* data)
            {
                auto* thisData = static_cast<SocketServerData* >(data);
                thisData->server->ProcessWorkerThreads(thisData);
            };
            JobSystem::RunJobs(&job, 1, nullptr);
            
            Debug::DebugColor(Debug::EStdoutColor::Green);
            {
                LogInfo.log("Skylar") << "## [^._.^] Skylar is now listening on "
                    << Settings.ResolvedAddressString << " (" << Listeners.GetSize() << " listeners).\n";
            }
            
            // list of new connections
            TArray<Net::INetSocket> socketsToAccept(Allocator);
            
            // Process receiving new connections
            do
            {
                PROFILE_FUNCTION()
                
                if (SocketsList.Accept(socketsToAccept))
                {
                    sockets.Mutex.Lock();
                    
                    for (uint32 i = 0; i < socketsToAccept.GetSize(); ++i)
                    {
                        const Net::INetSocket& socket = socketsToAccept[i];
                        
                        if (socket.IsOpen())
                        {
                            socket.MakeNonBlocking();
                            socket.SetSocketStreamNoDelay(true);
                            
                            sockets.Push(
                                         Tuple<Net::INetSocket, void* > { socket, nullptr });
                        }
                    }
                    
                    sockets.Mutex.Unlock();
                    
                    Controls.ProcessQueueEvent.Trigger();
                    
                    if (sockets.GetSize() >= Net::QueueMaxLength)
                    {
                        LogInfo.log("Skylar") << "Queue length " << Net::QueueMaxLength << " exceeded.";
                        QueueNotFullEvent.Reset();
                    }
                    
                    socketsToAccept.Clear();
                    
                    QueueNotFullEvent.Wait();
                }
            }
            while (Controls.Active or Controls.UpdateModulesEvent.Poll());
            
            LogInfo.log("Skylar") << "Server main cycle quit";
            
            // cleanup
            
            Controls.ProcessQueueEvent.Trigger();
            
            if (not Listeners.IsEmpty())
            {
                CloseListeners();
                Listeners.Clear();
            }
            
            Clear();
            
            return EXIT_SUCCESS;
        }

        
        // Task for processing incoming requests. @see Server::ProcessWorkerThreads
        class RequestTask : public MT::Task
        {
        public:
            
            RequestTask(Server& server, IAllocator& allocator, Net::SocketsQueue& sockets, MT::Event& threadRequestEvent)
            : MT::Task(allocator)
            , Sockets(sockets)
            , ThreadRequestEvent(threadRequestEvent)
            , QueueNotFullEvent(server.QueueNotFullEvent)
            , Settings(server.Settings)
            , Controls(server.Controls)
            , ThreadsWorkingCount(server.ThreadsWorkingCount)
            , TlsData(server.TlsData)
            , Allocator(allocator)
            {
            }
            
            IProtocol* CreateProto(ISocket& socket, void* stream, IAllocator& allocator) const
            {
                PROFILE_FUNCTION()
                
                IProtocol* protocol = nullptr;
  
                if (socket.GetTlsSession() != nullptr)
                {
                    void* session = socket.GetTlsSession();
                    
                    String protocolName(Allocator);
                    bool result = NegotiateProtocol(session, protocolName);
                    
                    if (result)
                    {
                        LogInfo.log("Skylar") << "Protocol negotiated as " << *protocolName;
                        
                        if (protocolName == "h2")
                        {
                            // @todo
                            protocol = nullptr;
                        }
                        else if (protocolName == "http/1.1")
                        {
                            protocol = NEKO_NEW(allocator, ProtocolHttp)(socket, &Settings, allocator);
                        }
                        
                        return protocol;
                    }
                    
//                    LogWarning.log("Skylar") << "Tls session data found, but couldn't negotiate needed protocol";
                }
                
                protocol = NEKO_NEW(allocator, ProtocolHttp)(socket, &Settings, allocator);
                
                return protocol;
            }
            
            void ThreadRequestProc(ISocket& socket, Net::SocketsQueue& sockets, void* stream) const
            {
                auto* protocol = CreateProto(socket, stream, Allocator);
                
                if (protocol)
                {
                    // Check if switching protocol
                    for (IProtocol* result = nullptr; ;)
                    {
                        // This may return a new instance if switching protocols
                        result = protocol->Process();
                        // ..so check
                        if (protocol == result)
                        {
                            break;
                        }
                        
                        NEKO_DELETE(Allocator, protocol);
                        protocol = result;
                    }
                    
                    protocol->Close();
                }
                
                NEKO_DELETE(Allocator, protocol);
            }
            
            virtual int32 DoTask() override
            {
                SetThreadDefaultAllocator(Allocator);
                
                while (true)
                {
                    PROFILE_SECTION("Server request process");
                    
                    Net::INetSocket socket;
                    void* streamData = nullptr; // @todo http/2
                    
                    ThreadRequestEvent.Wait();
                    
                    if (not Controls.Active)
                    {
                        break;
                    }
                    
                    Sockets.Mutex.Lock();
                    
                    // get socket and stream data
                    if (not Sockets.IsEmpty())
                    {
                        Tie(socket, streamData) = Sockets.front();
                        Sockets.Pop();
                    }
                    
                    if (Sockets.IsEmpty())
                    {
                        ThreadRequestEvent.Reset();
                        QueueNotFullEvent.Trigger();
                    }
                    
                    Sockets.Mutex.Unlock();
                    
                    if (socket.IsOpen())
                    {
                        ++ThreadsWorkingCount;
                        
                        // resolve
                        if (Net::NetAddress address; socket.GetAddress(address))
                        {
                            const uint16 port = address.Port;
                            
                            // it's a valid tls data, secured
                            if (auto it = TlsData.Find(port); it.IsValid())
                            {
                                auto* context = it.value();
                                assert(context != nullptr);
#   if USE_OPENSSL
                                SocketSSL socketSsl(socket, (SSL_CTX* )context);
#   endif
                                if (socketSsl.Handshake())
                                {
                                    ThreadRequestProc(socketSsl, Sockets, nullptr);
                                }
                            }
                            else
                            {
                                // use default socket
                                SocketDefault socketDefault(socket);
                                
                                ThreadRequestProc(socketDefault, Sockets, streamData);
                            }
                        }
                        --ThreadsWorkingCount;
                    }
                    else
                    {
                        // shouldn't happen
                    }
                }
                
                return EXIT_SUCCESS;
            }
            
        private:
            
            ServerSharedControls& Controls;
            ServerSettings& Settings;
            
            Net::SocketsQueue& Sockets;
            
            MT::Event& ThreadRequestEvent;
            MT::Event& QueueNotFullEvent;
            
            IAllocator& Allocator;
            
            Server::TlsMap& TlsData;
            
            ThreadSafeCounter& ThreadsWorkingCount;
        };
        
        uint16 Server::ProcessWorkerThreads(void* kek)
        {
            auto* data = static_cast<SocketServerData* >(kek);
            auto& sockets = static_cast<Net::SocketsQueue&>(*data->queue);
            
            ThreadsWorkingCount.Set(0);
            
            MT::Event threadsProcessEvent(true);
            
            const uint32& threadMaxCount = Settings.ThreadsMaxCount;
            
            // check thread count
            assert (threadMaxCount != 0);
            LogInfo.log("Skylar") << "Using " << threadMaxCount << " threads.";
            
            TArray<MT::Task*> activeTasks(Allocator);
            activeTasks.Reserve(threadMaxCount);
            
            // Update applications
            do
            {
                if (Controls.UpdateModulesEvent.Poll())
                {
                    UpdateApplications();
                }
                
                // process each application requests and threads
                do
                {
                    // create initial threads
                    while (activeTasks.GetSize() == ThreadsWorkingCount.GetValue()
                       and activeTasks.GetSize() < threadMaxCount and !sockets.IsEmpty())
                    {
                        auto* task = NEKO_NEW(Allocator, RequestTask)(*this, Allocator, sockets, threadsProcessEvent);
                        
                        StaticString<32> taskName("Skylar requests task #", ThreadsWorkingCount.GetValue() + 1);
                        
                        if (task->Create(taskName))
                        {
                            activeTasks.Push(task);
                        }
                    }
                    
                    uint32 notifyCount = activeTasks.GetSize() - ThreadsWorkingCount.GetValue();
                    
                    if (notifyCount > sockets.GetSize())
                    {
                        notifyCount = sockets.GetSize();
                    }
                    
                    threadsProcessEvent.Trigger(); // @todo notify only specified amount of threads?
                    
                    Controls.ProcessQueueEvent.Wait();
                }
                while (Controls.Active);
                
                // Cleanup
                
                threadsProcessEvent.Trigger();
                
                if (not activeTasks.IsEmpty())
                {
                    // cleanup threads
                    for (auto thread : activeTasks)
                    {
                        thread->Destroy();
                        NEKO_DELETE(Allocator, thread);
                    }
                    
                    activeTasks.Clear();
                }
                
                QueueNotFullEvent.Trigger();
            }
            while (Controls.UpdateModulesEvent.Poll(false));
            
            SocketsList.Destroy();
            
            return 0;
        }

        void Server::UpdateApplications()
        {
            PROFILE_FUNCTION()
            
            // Applications settings list
            TArray< ApplicationSettings* > applications(Allocator);
            // Get full applications settings list
            Settings.GetAllApplicationSettings(applications);
            
            TArray<uint32> updatedModules(Allocator);
            
            LogInfo.log("Skylar") << "Updating server applications..";
            
            for (const auto& application : applications)
            {
                const uint32 moduleIndex = application->ModuleIndex;
                
                // If module is not updated (not checked)
                if (not updatedModules.Contains(moduleIndex))
                {
                    // Check if update module is valid and loaded one isn't the same
                    if (not application->ServerModuleUpdatePath.IsEmpty()
                        and application->ServerModuleUpdatePath != application->ServerModulePath)
                    {
                        auto updateModuleStat = Neko::Platform::GetFileData(*application->ServerModuleUpdatePath);
                        auto updateModuleSize = updateModuleStat.FileSize;
                        auto updateModuleDate = updateModuleStat.ModificationTime;
                        
                        if (updateModuleStat.bIsValid)
                        {
                            updateModuleStat = Neko::Platform::GetFileData(*application->ServerModulePath);
                            auto moduleSize = updateModuleStat.FileSize;
                            auto moduleDate = updateModuleStat.ModificationTime;
                            
                            Module& module = Modules[moduleIndex];
                            
                            if (updateModuleStat.bIsValid)
                            {
                                if (moduleSize != updateModuleSize or moduleDate < updateModuleDate)
                                {
                                    UpdateApplication(module, applications, moduleIndex);
                                }
                            }
                        }
                    }
                    
                    updatedModules.Push(moduleIndex);
                }
            }
            
            LogInfo.log("Skylar") << "Application modules have been updated..";
            
            Controls.SetActiveFlag();
            Controls.UpdateModulesEvent.Reset();
        }
        
        bool Server::BindPort(const uint16 port, TArray<uint16>& ports)
        {
            if (ports.Contains(port))
            {
                LogError.log("Skylar") << "Attempt to bind socket with used port " << port << ".";
                return false;
            }
            
            // setup socket
            Net::INetSocket socket;
            Net::NetAddress address;
            
            address = socket.Init(*Settings.ResolvedAddressString, port, Net::ESocketType::TCP);
            
            if (address.AddressType == NA_BAD)
            {
                LogError.log("Skylar") << "Server couldn't start at " << Settings.ResolvedAddressString << ":" << port << ". " << strerror(errno);
                return false;
            }
            
            if (not socket.Bind(address))
            {
                LogError.log("Skylar") << "Server couldn't bind to address. " << strerror(errno);
                return false;
            }
            
            const int32 maxBacklog = SOMAXCONN;
            if (not socket.Listen(maxBacklog))
            {
                LogError.log("Skylar") << "Server couldn't be listen. " << strerror(errno);
                return false;
            }
            
            socket.MakeNonBlocking(true);
            socket.SetReuseAddress();
            socket.SetReusePort();
            
            Listeners.Emplace(Neko::Move(socket));
            
            ports.Push(port);
            
            return true;
        }
        
        bool Server::UpdateApplication(Module& module, TArray<ApplicationSettings* >& applications, const uint32 index)
        {
            TArray<ApplicationSettings* > existing(Allocator);
            
            for (auto& application : applications)
            {
                // all apps with the same module
                if (application->ModuleIndex == index)
                {
                    existing.Push(application);
                    
                    assert (application->OnApplicationExit);
                    {
                        application->OnApplicationExit();
                    }
                    
                    // @todo get rid of std function
                    application->OnApplicationInit = std::function<bool(ApplicationInitContext)>();
                    application->OnApplicationRequest = std::function<int16(Http::RequestData* , Http::ResponseData* )>();
                    application->OnApplicationPostRequest = std::function<void(Http::ResponseData* )>();
                    application->OnApplicationExit = std::function<void()>();
                }
            }
            
            module.Close();
            
            const auto application = *(existing.begin());
            
            const auto& moduleName = application->ServerModulePath;
            
            const int32 directoryPos = moduleName.Find("/");
            const int32 extensionPos = moduleName.Find(".");
            
            String moduleNameNew(Allocator);
            
            if (extensionPos != INDEX_NONE and (directoryPos == INDEX_NONE or directoryPos < extensionPos))
            {
                moduleNameNew = moduleName.Mid(0, extensionPos);
                moduleNameNew += Math::RandGUID();
                moduleNameNew += moduleName.Mid(extensionPos);
            }
            else
            {
                moduleNameNew = moduleName;
                moduleNameNew += Math::RandGUID();
            }
            
            FS::PlatformFile source;
            if (not source.Open(*application->ServerModuleUpdatePath, FS::Mode::Read))
            {
                LogError.log("Skylar") << "File '" << *application->ServerModuleUpdatePath << "' cannot be open";
                return false;
            }
            
            FS::PlatformFile destination;
            if (not destination.Open(*moduleNameNew, FS::Mode::CreateAndWrite))
            {
                LogError.log("Skylar") << "File '" << *moduleName << "' cannot be open";
                return false;
            }
            
            // Rewrite module file
            TArray<uint8> data(Allocator);
            data.Resize(source.GetSize());
            source.Read(&data[0], source.GetSize());
            destination.Write(&data[0], data.GetSize());
            
            source.Close();
            destination.Close();
            
            // Open updated module
            module.Open(moduleNameNew);
            
            if (not Neko::Platform::DeleteFile(*moduleName))
            {
                LogError.log("Skylar") << "File '" << *moduleName << "' could not be removed";
                return false;
            }
            
            if (not Neko::Platform::MoveFile(*moduleNameNew, *moduleName))
            {
                LogError.log("Skylar") << "Module '" << *moduleNameNew << "' could not be renamed";
                return false;
            }
            
            if (not module.IsOpen())
            {
                LogError.log("Skylar") << "Application module '" << *moduleName << "' can not be opened";
                return false;
            }
            
            // Set application module methods
            bool success = Settings.SetApplicationModuleMethods(*application, module);
            if (not success)
            {
                return false;
            }
            
            for (auto& app : existing)
            {
                app->OnApplicationInit = application->OnApplicationInit;
                app->OnApplicationRequest = application->OnApplicationRequest;
                app->OnApplicationPostRequest = application->OnApplicationPostRequest;
                app->OnApplicationExit = application->OnApplicationExit;
                
                assert (app->OnApplicationInit);
                
                ApplicationInitContext items
                {
                    *app->RootDirectory,
                    
                    &Allocator,
                    &FileSystem
                };
                
                app->OnApplicationInit(items);
            }
            
            return true;
        }

        void Server::CloseListeners()
        {
            for (auto& socket : Listeners)
            {
                socket.Close();
            }
        }
        
        void Server::Stop()
        {
            LogInfo.log("Skylar") << "Server is stopping..";
            
            QueueNotFullEvent.Trigger();
            
            Controls.StopProcess();
            CloseListeners();
        }
        
        void Server::Restart()
        {
            LogInfo.log("Skylar") << "Server is restarting..";
            
            Controls.SetRestartFlag();
            
            QueueNotFullEvent.Trigger();
            Controls.StopProcess();
            
            CloseListeners();
        }
        
        void Server::Update()
        {
            LogInfo.log("Skylar") << "Server is updating..";
            
            Controls.UpdateApplication();
            Controls.SetActiveFlag(false);
            Controls.ProcessQueue();
        }
        
        uint32 Server::GetServerProcessId(const String& serverName) const
        {
            uint32 processId = 0;
            {
                int memoryId;
                MT::SpinLock lick(Mutex);
                
                if ((memoryId = Neko::Platform::OpenSharedMemory(serverName)) != -1)
                {
                    Neko::Platform::ReadSharedMemory(memoryId, &processId, sizeof(processId));
                }
            }
            
            return processId;
        }
        
        // Commands
        
        uint16 Server::StartCommand(const String& name, bool force/* = false*/)
        {
            Neko::Platform::CheckSharedMemoryName((String&)name);
            
            if (force)
            {
                LogInfo.log("Skylar") << "Force server startup: " << *name;
                
                Neko::Platform::DestroySharedMemory(*name);
            }
            
            int memoryId;
            
            // Check if server is already running
            bool exists = false;
            {
                MT::SpinLock lock(Mutex);
                
                if ((memoryId = Neko::Platform::OpenSharedMemory(*name)) != -1 )
                {
                    int processId = 0;
                    
                    if (Neko::Platform::ReadSharedMemory(memoryId, &processId, sizeof(processId) ))
                    {
                        exists = Neko::Platform::ProcessExists(processId);
                    }
                }
            }
            
            if (exists)
            {
                LogError.log("Skylar") << "Server instance '" << *name << "' is already running!";
                
                return EXIT_FAILURE;
            }
            
            Mutex.Lock();
            
            if ((memoryId = Neko::Platform::OpenSharedMemory(*name)) == -1)
            {
                if ((memoryId = Neko::Platform::CreateSharedMemory(*name, sizeof(uint32))) == -1)
                {
                    Mutex.Unlock();
                    LogError.log("Skylar") << "Could not allocate shared memory!";
                    
                    return EXIT_FAILURE;
                }
            }
            
            const uint32 processId = Neko::Platform::GetCurrentProcessId();
            
            if (Neko::Platform::WriteSharedMemory(memoryId, &processId, sizeof(processId)) == false)
            {
                Neko::Platform::DestroySharedMemory(name);
                Mutex.Unlock();
                
                LogError.log("Skylar") << "Could not write data to shared memory!";
                
                return EXIT_FAILURE;
            }
            
            Mutex.Unlock();
            
            uint16 code = EXIT_FAILURE;
            
            do
            {
                Controls.SetActiveFlag(false);
                Controls.SetRestartFlag(false);
                
                code = Run();
            }
            while (Controls.Active or Controls.Restart);
            
            // cleanup
            Neko::Platform::DestroySharedMemory(name);
            
            return code;
        }
        
        uint16 Server::RestartCommand(const String& serverName) const
        {
            const uint32 processId = GetServerProcessId(serverName);
            
            return processId > 1 and Neko::Platform::SendSignal(processId, SIGUSR1) ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        
        uint16 Server::ExitCommand(const String& serverName) const
        {
            const uint32 processId = GetServerProcessId(serverName);
            
            return processId > 1 and Neko::Platform::SendSignal(processId, SIGTERM) ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        
        uint16 Server::UpdateModulesCommand(const String& serverName) const
        {
            const uint32 processId = GetServerProcessId(serverName);
            
            return processId > 1 and Neko::Platform::SendSignal(processId, SIGUSR2) ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        
        void Server::Clear()
        {
            QueueNotFullEvent.Reset();
            
            Controls.Clear();
            
            Settings.Clear();
            
            if (not TlsData.IsEmpty())
            {
                for (auto& item : TlsData)
                {
# if USE_OPENSSL
                    SSL_CTX_free((SSL_CTX* )item);
# endif
                }
                TlsData.Clear();
            }
            
            if (not Modules.IsEmpty())
            {
                for (auto& module : Modules)
                {
                    module.Close();
                }
                
                Modules.Clear();
            }
        }
    }
}

