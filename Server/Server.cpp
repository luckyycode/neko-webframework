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

#include "../../Engine/Core/Log.h"
#include "../../Engine/Core/Debug.h"
#include "../../Engine/Core/JobSystem.h"
#include "../../Engine/Mt/Task.h"
#include "../../Engine/Platform/Platform.h"
#include "../../Engine/Network/SocketQueue.h"
#include "../../Engine/Network/SocketList.h"
#include "../../Engine/FS/PlatformFile.h"

#include "../SocketSSL.h"
#include "../SocketDefault.h"

#include "Server.h"
#include "Http.h"
#include "IProtocol.h"

#include <signal.h> // commands

namespace Neko
{
    namespace Http
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
        
        // transient data
        struct SocketServerData
        {
            Server* server = nullptr;
            Net::SocketsQueue* queue = nullptr;
        };
        
        Server::Server(IAllocator& allocator, FS::FileSystem& fileSystem)
        : Mutex(false)
        , QueueNotFullEvent(true)
        , Allocator(allocator)
        , Modules(allocator)
        , Listeners(allocator)
        , TlsData(allocator)
        , SocketsList(Allocator)
        , Settings(fileSystem, allocator)
        {
            SetDefaultAllocator(allocator);
        };

        bool Server::Init()
        {
            GLogInfo.log("Http") << "Loading server settings";
            
            // load every application & settings
            bool settingsLoaded = Settings.LoadAppSettings("appsettings.json", Modules);
            
            if (!settingsLoaded)
            {
                return false;
            }
        
#   if USE_OPENSSL
            SocketSSL::Init();
#   endif

            return true;
        }
        
        void Server::Shutdown()
        {
            GLogInfo.log("Http") << "Server shutting down...";
            
            Stop();
            Clear();
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
        
        void* Server::InitSsl(const ApplicationSettings& application)
        {
#   if USE_OPENSSL
            SSL_CTX* context = nullptr;
            
            const auto& certificate = application.CertificateFile;
            const auto& privateKey = application.KeyFile;
            
            context = SSL_CTX_new(SSLv23_server_method());
            
            if (SSL_CTX_use_certificate_file(context, *certificate, SSL_FILETYPE_PEM) <= 0)
            {
                GLogError.log("Http") << "Couldn't load SSL certificate";
                goto cleanupSsl;
            }
            
            if (SSL_CTX_use_PrivateKey_file(context, privateKey.IsEmpty() ? *certificate : *privateKey, SSL_FILETYPE_PEM) <= 0)
            {
                GLogError.log("Http") << "Couldn't load SSL private key (or certificate pair)";
                goto cleanupSsl;
            }
            
            if (!SSL_CTX_check_private_key(context))
            {
                GLogError.log("Http") << "Couldn't verify SSL private key";
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
            GLogInfo.log("Http") << "Preparing server applications..";
            
            // Applications settings list
            TArray< ApplicationSettings* > applications(Allocator);
            
            // Get full applications settings list
            Settings.List.GetAllApplicationSettings(applications);
            
            // Bound port list
            TArray<uint32> ports(Allocator);
         
            // Open applications sockets
            for (auto& application : applications)
            {
                const uint32& tlsPort = application->TlsPort;
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
                
                GLogInfo.log("Http") << "Content directory: " << *application->RootDirectory;
            }
        }

        int32 Server::Run()
        {
            bool initialized = Init();
            if (!initialized)
            {
                return EXIT_FAILURE;
            }
            
            PrepareApplications();
            
            if (Listeners.IsEmpty())
            {
                Debug::DebugColor(Debug::EStdoutColor::Red); // aesthetics
                GLogError.log("Http") << "## Couldn't run server, no sockets opened.";
                Debug::DebugColor(Debug::EStdoutColor::White);
                
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
            
            GLogInfo.log("Http") << "Creating the main request queue task..";
            
            Net::SocketsQueue sockets(Allocator);
            
            // Create a job which will process all worker threads
            SocketServerData  data;
            data.server = this;
            data.queue = &sockets;
            
            JobSystem::JobDecl job;
            job.data = (void* )&data;
            job.task = [](void* data)
            {
                SocketServerData* thisData = (SocketServerData* )data;
                thisData->server->ProcessWorkerThreads(thisData);
            };
            JobSystem::RunJobs(&job, 1, nullptr);
            
            Debug::DebugColor(Debug::EStdoutColor::Green);
            GLogInfo.log("Http") << "## Server is now listening on " << Settings.ResolvedAddressString << " (" << Listeners.GetSize() << " listeners).";
            Debug::DebugColor(Debug::EStdoutColor::White);
            
            
            // list of new connections
            TArray<Net::INetSocket> socketsToAccept(Allocator);
            
            // Process receiving new connections
            do
            {
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
                        GLogInfo.log("Http") << "Queue length " << Net::QueueMaxLength << " exceeded.";
                        QueueNotFullEvent.Reset();
                    }
                    
                    socketsToAccept.Clear();
                    
                    QueueNotFullEvent.Wait();
                }
            }
            while (Controls.Active || Controls.UpdateModulesEvent.poll());
            
            GLogInfo.log("Http") << "Server main cycle quit";
            
            // cleanup
            
            Controls.ProcessQueueEvent.Trigger();
            
            if (!Listeners.IsEmpty())
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
                IProtocol* protocol = nullptr;
  
                if (socket.GetTlsSession() != nullptr)
                {
                    void* session = socket.GetTlsSession();
                    
                    String protocolName(Allocator);
                    bool result = NegotiateProtocol(session, protocolName);
                    
                    if (result)
                    {
                        GLogInfo.log("Http") << "Protocol negotiated as " << *protocolName;
                        
                        if (protocolName == "h2")
                        {
                            // @todo
                            protocol = NEKO_NEW(allocator, ProtocolHttp)(socket, &Settings, allocator);
                        }
                        else if (protocolName == "http/1.1")
                        {
                            protocol = NEKO_NEW(allocator, ProtocolHttp)(socket, &Settings, allocator);
                        }
                        
                        return protocol;
                    }
                    
//                    GLogWarning.log("Http") << "Tls session data found, but couldn't negotiate needed protocol";
                }
                
                protocol = NEKO_NEW(allocator, ProtocolHttp)(socket, &Settings, allocator);
                
                return protocol;
            }
            
            void ThreadRequestProc(ISocket& socket, Net::SocketsQueue& sockets, void* stream) const
            {
                IProtocol* protocol = CreateProto(socket, stream, Allocator);
                
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
                SetDefaultAllocator(Allocator);
                
                while (true)
                {
                    Net::INetSocket socket;
                    void* streamData = nullptr; // @todo http/2
                    
                    ThreadRequestEvent.Wait();
                    
                    if (!Controls.Active)
                    {
                        break;
                    }
                    
                    Sockets.Mutex.Lock();
                    
                    // get socket and stream data
                    if (!Sockets.IsEmpty())
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
                        
                        Net::NetAddress address;
                        bool success = socket.GetAddress(address);
                        if (success)
                        {
                            const uint32 port = address.Port;
                            auto it = TlsData.Find(port);
                            
                            // it's a valid tls data, secured
                            if (it.IsValid())
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
        
        int32 Server::ProcessWorkerThreads(void* kek)
        {
            SocketServerData* data = (SocketServerData* )kek;
            Net::SocketsQueue& sockets = *(Net::SocketsQueue*)data->queue;
            
            ThreadsWorkingCount.Set(0);
            
            MT::Event threadsProcessEvent(true);
            
            const uint32& threadMaxCount = Settings.ThreadsMaxCount;
            
            // check thread count
            assert (threadMaxCount != 0);
            GLogInfo.log("Http") << "Using " << threadMaxCount << " threads.";
            
            TArray<MT::Task*> activeTasks(Allocator);
            activeTasks.Reserve(threadMaxCount);
            
            // Update applications
            do
            {
                if (Controls.UpdateModulesEvent.poll())
                {
                    UpdateApplications();
                }
                
                // process each application requests and threads
                do
                {
                    // create initial threads
                    while (activeTasks.GetSize() == ThreadsWorkingCount.GetValue()
                           && activeTasks.GetSize() < threadMaxCount && !sockets.IsEmpty())
                    {
                        RequestTask* task = NEKO_NEW(Allocator, RequestTask)(*this, Allocator, sockets, threadsProcessEvent);
                        if (task->Create("Server requests task"))
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
                
                if (!activeTasks.IsEmpty())
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
            while (Controls.UpdateModulesEvent.poll(false));
            
            SocketsList.Destroy();
            
            return 0;
        }

        void Server::UpdateApplications()
        {
            // Applications settings list
            TArray< ApplicationSettings* > applications(Allocator);
            // Get full applications settings list
            Settings.List.GetAllApplicationSettings(applications);
            
            TArray<uint32> updatedModules(Allocator);
            
            GLogInfo.log("Http") << "Updating server applications..";
            
            for (const auto& application : applications)
            {
                const uint32 moduleIndex = application->ModuleIndex;
                
                // If module is not updated (not checked)
                if (!updatedModules.Contains(moduleIndex))
                {
                    // Check if update module is valid and loaded one isn't the same
                    if (!application->ServerModuleUpdate.IsEmpty()
                        && application->ServerModuleUpdate != application->ServerModule)
                    {
                        auto updateModuleStat = Neko::Platform::GetFileData(*application->ServerModuleUpdate);
                        auto updateModuleSize = updateModuleStat.FileSize;
                        auto updateModuleDate = updateModuleStat.ModificationTime;
                        
                        if (updateModuleStat.bIsValid)
                        {
                            updateModuleStat = Neko::Platform::GetFileData(*application->ServerModule);
                            auto moduleSize = updateModuleStat.FileSize;
                            auto moduleDate = updateModuleStat.ModificationTime;
                            
                            Module& module = Modules[moduleIndex];
                            
                            if (updateModuleStat.bIsValid)
                            {
                                if (moduleSize != updateModuleSize || moduleDate < updateModuleDate)
                                {
                                    UpdateApplication(module, applications, moduleIndex);
                                }
                            }
                        }
                    }
                    
                    updatedModules.Push(moduleIndex);
                }
            }
            
            GLogInfo.log("Http") << "Application modules have been updated..";
            
            Controls.SetActiveFlag();
            Controls.UpdateModulesEvent.Reset();
        }
        
        bool Server::BindPort(const uint32 port, TArray<uint32>& ports)
        {
            if (ports.Contains(port))
            {
                GLogError.log("Http") << "Attempt to bind socket with used port " << port << ".";
                return false;
            }
            
            // setup socket
            Net::INetSocket socket;
            
            if (!socket.Init(*Settings.ResolvedAddressString, port, Net::ESocketType::TCP))
            {
                GLogError.log("Http") << "Server couldn't start at " << Settings.ResolvedAddressString << ":" << port << ". " << strerror(errno);
                return false;
            }
            
            if (!socket.Bind())
            {
                GLogError.log("Http") << "Server couldn't bind to address. " << strerror(errno);
                return false;
            }
            
            const int32 maxBacklog = SOMAXCONN;
            if (!socket.Listen(maxBacklog))
            {
                GLogError.log("Http") << "Server couldn't be listen. " << strerror(errno);
                return false;
            }
            
            socket.MakeNonBlocking(true);
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
                    application->OnApplicationInit = std::function<bool(ApplicationInitDesc)>();
                    application->OnApplicationRequest = std::function<int(Net::Http::RequestData* , Net::Http::ResponseData* )>();
                    application->OnApplicationPostRequest = std::function<void(Net::Http::ResponseData* )>();
                    application->OnApplicationExit = std::function<void()>();
                }
            }
            
            module.Close();
            
            const auto application = *(existing.begin());
            
            const String& moduleName = application->ServerModule;
            
            const int32 directoryPos = moduleName.Find("/");
            const int32 extensionPos = moduleName.Find(".");
            
            String moduleNameNew(Allocator);
            
            if (extensionPos != INDEX_NONE && (directoryPos == INDEX_NONE || directoryPos < extensionPos))
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
            
            FS::CPlatformFile source;
            if (!source.Open(*application->ServerModuleUpdate, FS::Mode::READ))
            {
                GLogError.log("Http") << "File '" << *application->ServerModuleUpdate << "' cannot be open";
                return false;
            }
            
            FS::CPlatformFile destination;
            if (!destination.Open(*moduleNameNew, FS::Mode::CREATE_AND_WRITE))
            {
                GLogError.log("Http") << "File '" << *moduleName << "' cannot be open";
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
            
            if (!Neko::Platform::DeleteFile(*moduleName))
            {
                GLogError.log("Http") << "File '" << *moduleName << "' could not be removed";
                return false;
            }
            
            if (!Neko::Platform::MoveFile(*moduleNameNew, *moduleName))
            {
                GLogError.log("Http") << "Module '" << *moduleNameNew << "' could not be renamed";
                return false;
            }
            
            if (!module.IsOpen())
            {
                GLogError.log("Http") << "Application module '" << *moduleName << "' can not be opened";
                return false;
            }
            
            // Set application module methods
            bool success = Settings.SetApplicationModuleMethods(*application, module);
            if (!success)
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
                
                ApplicationInitDesc items
                {
                    *app->RootDirectory
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
            GLogInfo.log("Http") << "Server is stopping..";
            
            QueueNotFullEvent.Trigger();
            
            Controls.StopProcess();
            CloseListeners();
        }
        
        void Server::Restart()
        {
            GLogInfo.log("Http") << "Server is restarting..";
            
            Controls.SetRestartFlag();
            
            QueueNotFullEvent.Trigger();
            Controls.StopProcess();
            
            CloseListeners();
        }
        
        void Server::Update()
        {
            GLogInfo.log("Http") << "Server is updating..";
            
            Controls.UpdateApplication();
            Controls.SetActiveFlag(false);
            Controls.ProcessQueue();
        }
        
        int32 Server::GetServerProcessId(const String& serverName) const
        {
            int processId = 0;
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
        
        int Server::StartCommand(const String& name, bool force/* = false*/)
        {
            Neko::Platform::CheckSharedMemoryName((String&)name);
            
            if (force)
            {
                GLogInfo.log("Http") << "Force server startup: " << *name;
                
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
                GLogError.log("Http") << "Server instance '" << *name << "' is already running!";
                
                return EXIT_FAILURE;
            }
            
            Mutex.Lock();
            
            if ((memoryId = Neko::Platform::OpenSharedMemory(*name)) == -1)
            {
                if ((memoryId = Neko::Platform::CreateSharedMemory(*name, sizeof(uint32))) == -1)
                {
                    Mutex.Unlock();
                    GLogError.log("Http") << "Could not allocate shared memory!";
                    
                    return EXIT_FAILURE;
                }
            }
            
            const uint32 processId = Neko::Platform::GetCurrentProcessId();
            
            if (Neko::Platform::WriteSharedMemory(memoryId, &processId, sizeof(processId)) == false)
            {
                Neko::Platform::DestroySharedMemory(name);
                Mutex.Unlock();
                
                GLogError.log("Http") << "Could not write data to shared memory!";
                
                return EXIT_FAILURE;
            }
            
            Mutex.Unlock();
            
            int32 code = EXIT_FAILURE;
            
            do
            {
                Controls.SetActiveFlag(false);
                Controls.SetRestartFlag(false);
                
                code = Run();
            }
            while (Controls.Active || Controls.Restart);
            
            // cleanup
            Neko::Platform::DestroySharedMemory(name);
            
            return EXIT_SUCCESS;
        }
        
        int Server::RestartCommand(const String& serverName) const
        {
            const uint32 processId = GetServerProcessId(serverName);
            
            return processId > 1 && Neko::Platform::SendSignal(processId, SIGUSR1) ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        
        int Server::ExitCommand(const String& serverName) const
        {
            const uint32 processId = GetServerProcessId(serverName);
            
            return processId > 1 && Neko::Platform::SendSignal(processId, SIGTERM) ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        
        int Server::UpdateModulesCommand(const String& serverName) const
        {
            const uint32 processId = GetServerProcessId(serverName);
            
            return processId > 1 && Neko::Platform::SendSignal(processId, SIGUSR2) ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        
        void Server::Clear()
        {
            QueueNotFullEvent.Reset();
            
            Controls.Clear();
            
            Settings.Clear();
            
            if (!TlsData.IsEmpty())
            {
                for (auto& item : TlsData)
                {
# if USE_OPENSSL
                    SSL_CTX_free((SSL_CTX* )item);
# endif
                }
                TlsData.Clear();
            }
            
            if (!Modules.IsEmpty())
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

