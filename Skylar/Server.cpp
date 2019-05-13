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
//
//  Server.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Engine/Core/Log.h"
#include "Engine/Core/Debug.h"
#include "Engine/Core/Async.h"
#include "Engine/Core/Profiler.h"
#include "Engine/Mt/Task.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Platform/SocketPooler.h"
#include "Engine/Network/SocketPool.h"
#include "Engine/FileSystem/PlatformFile.h"

#include "Server.h"
#include "Http.h"
#include "ISsl.h"
#include "Protocol.h"
#include "RequestPoolHandler.h"

#include "../Sockets/SocketSSL.h"
#include "../Sockets/SocketDefault.h"

#include <signal.h> // commands
#include <thread>

namespace Neko::Skylar
{
    using namespace Neko::FileSystem;
    static const char* ServerSettingsFileName = "serversettings.json";

    Server::Server(IAllocator& allocator, IFileSystem& fileSystem)
        : Mutex(false)
        , QueueNotFullEvent(true)
        , Sockets(allocator)
        , Allocator(allocator)
        , Modules(allocator)
        , Listeners(allocator)
        , Pooler(allocator)
        , FileSystem(fileSystem)
        , Settings(fileSystem, allocator)
    {
        SetThreadDefaultAllocator(allocator);
    };
    
    bool Server::Init()
    {
        LogInfo.log("Skylar") << "Server initializing..";
        
        // load every application & settings
        if (not Settings.LoadAppSettings(ServerSettingsFileName, Modules))
        {
            return false;
        }
        
        this->Ssl = ISsl::Create(Allocator);
        LogInfo.log("Skylar") << "Server initialization complete.";

        return true;
    }
    
    void Server::Shutdown()
    {
        LogInfo.log("Skylar") << "Server shutting down...";
        
        Stop();
        Clear();
        
        if (this->Ssl != nullptr)
        {
            ISsl::Destroy(*this->Ssl);
        }
    }
    
    void* Server::InitSslFor(const PoolApplicationSettings& application)
    {
        assert(this->Ssl != nullptr);
        return this->Ssl->InitSslFor(application);
    }
    
    void Server::PrepareApplications()
    {
        LogInfo.log("Skylar") << "Preparing server applications..";
        // applications settings list
        TArray< PoolApplicationSettings* > applications(Allocator);
        // get full applications settings list
        Settings.GetAllApplicationSettings(applications);
        // bound port list
        TArray<uint16> ports(Allocator);
        
        // open applications sockets
        for (auto& application : applications)
        {
            const uint16 tlsPort = application->TlsPort;
            const uint16 port = application->Port;

            // init ssl Data for this port
            if (tlsPort != 0)
            {
                // initialize it first
                auto* context = InitSslFor(*application);
                if (context != nullptr && BindPort(tlsPort, ports))
                {
                    // save context
                    this->Ssl->AddSession(tlsPort, context);
                }
            }

            // init default port
            if (port != 0)
            {
                BindPort(port, ports);
            }

            LogInfo.log("Skylar") << "Content directory: " << *application->RootDirectory;
        }
    }

    uint16 Server::Run()
    {
        if (not Init())
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
        Pooler.Create((uint32) Listeners.GetSize());
        
        // Init list with created sockets
        for (const auto& socket : Listeners)
        {
            Pooler.AddSocket(socket);
        }
        
        // start processing immediately
        Controls.SetActiveFlag();
        LogInfo.log("Skylar") << "Creating the main request task..";

        struct RequestProcessTask final : public Neko::Sync::Task
        {
            RequestProcessTask(Neko::IAllocator& Allocator, Skylar::Server* server)
                : Task(Allocator), Server(server) { }

            int32 DoTask() { return Server->ProcessWorkerThreads(); }

            Server* Server;
        };

        // create a task which will process all worker threads
        auto* task = NEKO_NEW(Allocator, RequestProcessTask) (Allocator, this);
        if (not task->Create("Request Handler Instance"))
        {
            return EXIT_FAILURE;
        }
        
        Debug::PrintColor(Debug::StdoutColor::Green);
        {
            LogInfo.log("Skylar") << "## [^._.^] Skylar is now listening on "
                << Settings.ResolvedAddressString << " (" << Listeners.GetSize() << " listeners).\n";
        }
        
        // a list of new connections
        TArray<Net::NetSocketBase> socketsToAccept(Allocator);
        socketsToAccept.Reserve(128);
        // process receiving new connections
        do
        {
            PROFILE_FUNCTION();
            if (Pooler.Accept(socketsToAccept)) // blocking
            {
                Sockets.CriticalSection.Enter();
                
                for (const auto& socket : socketsToAccept)
                {
                    if (not socket.IsOpen())
                    {
                        continue;
                    }

                    socket.MakeNonBlocking();
                    socket.SetSocketStreamNoDelay(true);

                    Sockets.Push(Tuple<Net::NetSocketBase, void* > { socket, nullptr });
                }

                Sockets.CriticalSection.Exit();
                Controls.ProcessQueueSemaphore.Signal();

                if (Sockets.GetSize() >= Net::PoolMaxLength)
                {
                    LogInfo.log("Skylar") << "Pool length " << Net::PoolMaxLength << " exceeded.";
                    QueueNotFullEvent.Reset();
                }
                
                socketsToAccept.Clear();
                QueueNotFullEvent.Wait();
            }
        }
        while (Controls.Active or Controls.UpdateModulesEvent.Poll());
        
        LogInfo.log("Skylar") << "Server main cycle quit";
        
        // cleanup
        Controls.ProcessQueueSemaphore.Signal();
        if (not Listeners.IsEmpty())
        {
            CloseListeners();
            Listeners.Clear();
        }

        Clear();
        NEKO_DELETE(Allocator, task);

        return EXIT_SUCCESS;
    }

    uint16 Server::ProcessWorkerThreads()
    {
        Sync::Event event(false, true); // event
        RequestPoolHandler requestPool(*this, Sockets, &event);
        // update applications
        do
        {
            // handle application module update in a background
            if (Controls.UpdateModulesEvent.Poll())
            {
                UpdateApplications();
            }

            // process application requests
            do
            {
                requestPool.Handle();

                event.Notify();
                Controls.ProcessQueueSemaphore.Wait();
            }
            while (Controls.Active);
            
            // Cleanup
            event.Notify();

            // todo possibly wait for task completion here
            QueueNotFullEvent.Notify();
        }
        while (Controls.UpdateModulesEvent.Poll(false));
        
        Pooler.Destroy();
        return EXIT_SUCCESS;
    }
    
    void Server::UpdateApplications()
    {
        PROFILE_FUNCTION();
        
        // Applications settings list
        TArray< PoolApplicationSettings* > applications(Allocator);
        // Get full applications settings list
        Settings.GetAllApplicationSettings(applications);
        
        TArray<int32> updatedModules(Allocator);
        LogInfo.log("Skylar") << "Updating server applications..";
        
        for (const auto& application : applications)
        {
            const int32 moduleIndex = application->ModuleIndex;

            if (moduleIndex == INDEX_NONE)
            {
                LogWarning.log("Skylar") << "Module update skipped for " << application->ServerModuleUpdatePath;
                continue;
            }

            // If module is not updated (not checked)
            if (not updatedModules.Contains(moduleIndex))
            {
                // Check if update module is valid and loaded one isn't the same
                if (not application->ServerModuleUpdatePath.IsEmpty()
                    and application->ServerModuleUpdatePath != application->ServerModulePath)
                {
                    auto updateModuleStat = Neko::Platform::GetFileData(*application->ServerModuleUpdatePath);
                    auto updateModuleDate = updateModuleStat.ModificationTime;
                    int64 updateModuleSize = updateModuleStat.FileSize;
                    
                    if (updateModuleStat.IsValid)
                    {
                        updateModuleStat = Neko::Platform::GetFileData(*application->ServerModulePath);
                        int64 moduleSize = updateModuleStat.FileSize;
                        auto moduleDate = updateModuleStat.ModificationTime;
                        
                        auto& module = Modules[moduleIndex];

                        if (updateModuleStat.IsValid && (moduleSize != updateModuleSize or moduleDate < updateModuleDate))
                        {
                            UpdateApplication(module, applications, moduleIndex);
                        }
                    }
                }
                updatedModules.Push(moduleIndex);
            }
        }
        
        LogInfo.log("Skylar") << "Application modules have been updated.";
        
        Controls.SetActiveFlag();
        Controls.UpdateModulesEvent.Reset();
    }
    
    bool Server::BindPort(const uint16 port, TArray<uint16>& ports)
    {
        if (ports.Contains(port))
        {
            LogError.log("Skylar") << "Attempt to bind a socket with the used port " << port << ".";

            return false;
        }
        
        // setup socket
        Net::NetSocketBase socket;
        Net::Endpoint address;
        
        address = socket.Init(*Settings.ResolvedAddressString, port, Net::SocketType::Tcp);
        socket.SetLinger();

        const int32 maxBacklog = SOMAXCONN;

        if (address.AddressType == NetworkAddressType::Incorrect
            or not socket.Bind(address)
            or not socket.Listen(maxBacklog))
        {
            LogError.log("Skylar") << "Server couldn't start at " << Settings.ResolvedAddressString << ":" << port << ". "
                << *Platform::GetLastErrorMessage();

            return false;
        }
        
        socket.MakeNonBlocking(true);
        socket.SetReuseAddress();
        socket.SetReusePort();
        
        Listeners.Emplace(Neko::Move(socket));
        
        ports.Push(port);
        return true;
    }
    
    bool Server::UpdateApplication(Module& module, TArray<PoolApplicationSettings* >& applications, const uint32 index)
    {
        TArray<PoolApplicationSettings* > existing(Allocator);
        
        for (auto& application : applications)
        {
            // all apps with the same module
            if (application->ModuleIndex == index)
            {
                existing.Push(application);
                
                assert(application->OnApplicationExit != nullptr);
                application->OnApplicationExit();
                
                application->OnApplicationInit = std::function<bool(ApplicationInitContext)>();
                application->OnApplicationRequest = std::function<int16(Http::RequestData* , Http::ResponseData* )>();
                application->OnApplicationPostRequest = std::function<void(Http::ResponseData* )>();
                application->OnApplicationExit = std::function<void()>();
            }
        }
        
        module.Close();
        
        const auto application = *(existing.begin());
        const String& moduleName = application->ServerModulePath;
        
        const int32 directoryPos = moduleName.Find("/");
        const int32 extensionPos = moduleName.Find(".");
        
        String moduleNameNew(Allocator);
        
        if (extensionPos != INDEX_NONE and (directoryPos == INDEX_NONE or directoryPos < extensionPos))
        {
            moduleNameNew = moduleName.Mid(0, extensionPos);
            moduleNameNew += Math::RandGuid();
            moduleNameNew += moduleName.Mid(extensionPos);
        }
        else
        {
            moduleNameNew = moduleName;
            moduleNameNew += Math::RandGuid();
        }
        
        FileSystem::PlatformFile source;
        if (not source.Open(*application->ServerModuleUpdatePath, FileSystem::Mode::Read))
        {
            LogError.log("Skylar") << "File \"" << *application->ServerModuleUpdatePath << "\" cannot be open.";

            return false;
        }
        
        FileSystem::PlatformFile destination;
        if (not destination.Open(*moduleNameNew, FileSystem::Mode::CreateAndWrite))
        {
            LogError.log("Skylar") << "File \"" << *moduleName << "\" cannot be open.";

            return false;
        }

        // rewrite a module file
        {
            TArray<uint8> data(Allocator);
            data.Resize(source.GetSize());
            source.Read(&data[0], source.GetSize());
            destination.Write(&data[0], data.GetSize());

            source.Close();
            destination.Close();
        }

        // Open updated module
        module.Open(moduleNameNew);
        
        if (not Neko::Platform::DeleteFile(*moduleName))
        {
            LogError.log("Skylar") << "File '" << *moduleName << "' can not be removed.";

            return false;
        }
        
        if (not Neko::Platform::MoveFile(*moduleNameNew, *moduleName))
        {
            LogError.log("Skylar") << "Module '" << *moduleNameNew << "' can not be renamed.";

            return false;
        }
        
        if (not module.IsOpen())
        {
            LogError.log("Skylar") << "Application module '" << *moduleName << "' can not be opened";

            return false;
        }
        
        // set application module methods
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
            
            assert(app->OnApplicationInit);
            
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
        LogInfo.log("Skylar") << "Closing " << Listeners.GetSize() << " listeners..";

        for (auto& socket : Listeners)
        {
            socket.Close();
        }
    }
    
    void Server::Stop()
    {
        LogInfo.log("Skylar") << "Server is stopping..";

        QueueNotFullEvent.Notify();
        
        Controls.StopProcess();
        CloseListeners();
    }
    
    void Server::Restart()
    {
        LogInfo.log("Skylar") << "Server is restarting..";
        
        Controls.SetRestartFlag();

        QueueNotFullEvent.Notify();
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
            Sync::SpinLock lick(Mutex);
            
            if ((memoryId = Neko::Platform::OpenSharedMemory(serverName)) != -1)
            {
                Neko::Platform::ReadSharedMemory(memoryId, &processId, sizeof(processId));
            }
        }
        
        return processId;
    }
    
    // Commands
    
    uint16 Server::StartCommand(String& mutableName, bool force/* = false*/)
    {
        Neko::Platform::CheckSharedMemoryName(mutableName);
        
        if (force)
        {
            LogInfo.log("Skylar") << "Force server startup: " << *mutableName;
            Neko::Platform::DestroySharedMemory(*mutableName);
        }
        
        int memoryId;
        
        // Check if server is already running
        bool exists = false;
        {
            Sync::SpinLock lock(Mutex);

            uint32 processId = 0;
            if ((memoryId = Neko::Platform::OpenSharedMemory(*mutableName)) != -1
                && Neko::Platform::ReadSharedMemory(memoryId, &processId, sizeof(processId)))
            {
                exists = Neko::Platform::ProcessExists(processId);
            }
        }
        
        if (exists)
        {
            LogError.log("Skylar") << "Server instance \"" << *mutableName << "\" is already running!";

            return EXIT_FAILURE;
        }
        
        Mutex.Lock();
        
        if ((memoryId = Neko::Platform::OpenSharedMemory(*mutableName)) == -1)
        {
            if ((memoryId = Neko::Platform::CreateSharedMemory(*mutableName, sizeof(uint32))) == -1)
            {
                Mutex.Unlock();
                LogError.log("Skylar") << "Could not allocate shared memory!";

                return EXIT_FAILURE;
            }
        }
        
        const uint32 processId = Neko::Platform::GetCurrentProcessId();
        
        if (Neko::Platform::WriteSharedMemory(memoryId, &processId, sizeof(processId)) == false)
        {
            Neko::Platform::DestroySharedMemory(mutableName);
            Mutex.Unlock();
            LogError.log("Skylar") << "Could not write Data to shared memory!";

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
        Neko::Platform::DestroySharedMemory(mutableName);

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
        
        if (Ssl != nullptr)
        {
            Ssl->Clear();
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


