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
//  Server.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Core/Module.h"
#include "Engine/Data/IAllocator.h"
#include "Engine/Utilities/Tuple.h"
#include "Engine/Network/SocketPool.h"
#include "Engine/Platform/SocketPooler.h"

#include "CycleManager.h"
#include "../SharedSettings.h"

#include "../Tls.h"
#include "IServer.h"

namespace Neko::Skylar
{
    static const char* DEFAULT_SERVER_NAME = "Skylar";

    /** Server instance. */
    class Server : public IServer
    {
    public:
        Server(IAllocator& allocator, class FileSystem::IFileSystem& fileSystem);

        virtual ~Server() { }

        /** Prepares applications. Binds found application ports. */
        void PrepareApplications();
        
        /** Manages worker threads. */
        uint16 ProcessWorkerThreads();

        IAllocator& GetAllocator() override
        {
            return Allocator;
        }

        /** Clears cached data and settings. */
        void Clear();
        
        /** Gets server process id (if multiple are running). */
        uint32 GetServerProcessId(const String& serverName) const override;

    private:
        bool Init() override;
        uint16 Run() override;
        void Shutdown() override;

        bool BindPort(const uint16 port, TArray<uint16>& ports) override;

        /** Updates all server modules. */
        void UpdateApplications();
        
        /** Updates the specified server module. */
        bool UpdateApplication(Module& module, TArray< PoolApplicationSettings* >& applications, const uint32 moduleIndex);

        void* InitSslFor(const PoolApplicationSettings& application);
        void CloseListeners();
        
    public:
        // commands
        void Stop();
        void Restart();
        void Update();
        
        uint16 StartCommand(String& mutableName, bool force = false);
        uint16 RestartCommand(const String& serverName) const;
        uint16 ExitCommand(const String& serverName) const;
        uint16 UpdateModulesCommand(const String& serverName) const;
        
    protected:
        /** Shared server settings */
        ServerSharedSettings Settings;
        
    public:
        /** Shared external server controls. */
        mutable CycleManager Controls;
        
    protected:
        SocketPooler Pooler;
        
        /** List of active sockets which listen for incoming connections. */
        TArray< Net::NetSocketBase > Listeners;
        /** Loaded server modules. */
        TArray< Module > Modules;
        
        Sync::Event QueueNotFullEvent;
        
        /** Global server mutex. */
        mutable Sync::SpinMutex Mutex;

        class ISsl* Ssl;
        
    public:
        Net::SocketPool Sockets;

    private:
        IAllocator& Allocator;
        FileSystem::IFileSystem& FileSystem;
        
        friend class RequestPoolHandler;
    private:
        NON_COPYABLE(Server)
    };
}

