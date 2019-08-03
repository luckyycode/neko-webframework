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
#include "ConnectionDispatcher.h"

#include "../Sockets/SocketSSL.h"
#include "../Sockets/SocketDefault.h"

#include <signal.h> // commands
#include <thread>
#include <Engine/Network/NetSocketBuilder.h>

#include "SkylarOptions.h"

namespace Neko::Skylar
{
    using namespace Neko::FileSystem;

    Server::Server(IAllocator& allocator, IFileSystem& fileSystem)
        : Mutex(false)
        , QueueNotFullEvent(true)
        , Sockets(allocator)
        , Allocator(allocator)
        , Transports(allocator)
        , Pooler(allocator)
        , FileSystem(fileSystem)
        , Modules(*this, allocator, fileSystem)
        , Options(allocator)
        , Binder(allocator)
        , Role(ServerRole::ReverseProxy)
    {
        SetThreadDefaultAllocator(allocator);
    };
    
    bool Server::Init()
    {
        LogInfo("Skylar") << "Skylar is initializing..";
        Modules.LoadSettings();

        return true;
    }
    
    void Server::Shutdown()
    {
        LogInfo("Skylar") << "Server shutting down...";
        
        Stop();
        Clear();
    }

    uint16 Server::Run()
    {
        if (not Init())
        {
            return EXIT_FAILURE;
        }
        
        Modules.PrepareApplications();

        Binder.Bind(Options,
            [&](ListenOptions& options, NetSocketBase* transport) mutable
            {
                Transports.Push(transport);
            });

        if (Transports.IsEmpty())
        {
            // aesthetics
            LogError("Skylar") << "## (⌒_⌒;) Couldn't run Skylar, no transport were opened.";
            Clear();
            
            return EXIT_FAILURE;
        }
        
        // Init sockets
        Pooler.Create((uint32) Transports.GetSize());
        // Init list with created sockets
        for (const auto* socket : Transports)
        {
            Pooler.AddSocket(*socket);
        }

        // start processing immediately
        Controls.SetActiveFlag();
        LogInfo("Skylar") << "Creating the main request task..";

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
            LogInfo("Skylar") << "## [^._.^] Skylar is now listening. " << " (" << Transports.GetSize() << " listeners).\n";
        }
        
        // a list of new connections
        TArray<Net::NetSocketBase> socketsToAccept(Allocator);
        socketsToAccept.Reserve(SOMAXCONN);
        // process receiving new connections
        do
        {
            // low level processing, connections are handled in the connectables
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

                    Sockets.Push(SocketConnection(socket, nullptr));
                }

                Sockets.CriticalSection.Exit();
                Controls.ProcessQueueSemaphore.Signal();

                if (Sockets.GetSize() >= Net::PoolMaxLength)
                {
                    LogInfo("Skylar") << "Pool length " << Net::PoolMaxLength << " exceeded.";
                    QueueNotFullEvent.Reset();
                }
                
                socketsToAccept.Clear();
                QueueNotFullEvent.Wait();
            }
        }
        while (Controls.Active or Controls.UpdateModulesEvent.Poll());
        
        LogInfo("Skylar") << "Server main cycle quit";
        
        // cleanup
        Controls.ProcessQueueSemaphore.Signal();
        if (not Transports.IsEmpty())
        {
            CloseListeners();
            Transports.Clear();
        }

        Clear();
        NEKO_DELETE(Allocator, task);

        return EXIT_SUCCESS;
    }

    uint16 Server::ProcessWorkerThreads()
    {
        MiddlewareDelegate connectionDelegate;

        Sync::Event event(false, true);
        ConnectionDispatcher connectionDispatcher(*this, Sockets, &event, connectionDelegate);
        // update applications
        do
        {
            // handle application module update in a background
            if (Controls.UpdateModulesEvent.Poll())
            {
                Modules.UpdateApplications();
            }

            // process application requests
            do
            {
                connectionDispatcher.Handle();

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
        
        connectionDispatcher.Complete();
        Pooler.Destroy();
        
        return EXIT_SUCCESS;
    }
    
    void Server::CloseListeners()
    {
        LogInfo("Skylar") << "Closing " << Transports.GetSize() << " listeners..";

        for (auto* socket : Transports)
        {
            socket->Close();
            NEKO_DELETE(Allocator, socket);
        }

        Binder.Destroy();
    }
    
    void Server::Stop()
    {
        LogInfo("Skylar") << "Server is stopping..";

        QueueNotFullEvent.Notify();
        
        Controls.StopProcess();
        CloseListeners();
    }
    
    void Server::Restart()
    {
        LogInfo("Skylar") << "Server is restarting..";
        
        Controls.SetRestartFlag();

        QueueNotFullEvent.Notify();
        Controls.StopProcess();
        
        CloseListeners();
    }
    
    void Server::Update()
    {
        LogInfo("Skylar") << "Server is updating..";
        
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
            LogInfo("Skylar") << "Force server startup: " << *mutableName;
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
            LogError("Skylar") << "Server instance \"" << *mutableName << "\" is already running!";

            return EXIT_FAILURE;
        }
        
        Mutex.Lock();
        
        if ((memoryId = Neko::Platform::OpenSharedMemory(*mutableName)) == -1)
        {
            if ((memoryId = Neko::Platform::CreateSharedMemory(*mutableName, sizeof(uint32))) == -1)
            {
                Mutex.Unlock();
                LogError("Skylar") << "Could not allocate shared memory!";

                return EXIT_FAILURE;
            }
        }
        
        const uint32 processId = Neko::Platform::GetCurrentProcessId();
        
        if (Neko::Platform::WriteSharedMemory(memoryId, &processId, sizeof(processId)) == false)
        {
            Neko::Platform::DestroySharedMemory(mutableName);
            Mutex.Unlock();
            LogError("Skylar") << "Could not write Data to shared memory!";

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

        Modules.Cleanup();
        Binder.Clear();
    }
}


