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
#include "ServerState.h"
#include "ModuleManager.h"

#include "../Tls.h"
#include "IServer.h"
#include "ListenOptions.h"
#include "SkylarOptions.h"
#include "AddressBinder.h"

namespace Neko::Skylar
{
    static const char* DEFAULT_SERVER_NAME = "Skylar";

    enum struct ServerRole : uint8
    {
        /** Internet-facing mode */
        Edge = 0x0,
        
        /** Reverse proxy role */
        ReverseProxy = 0x1,
    };
    
    /** Server instance. */
    class Server : public IServer
    {
    public:
        Server(IAllocator& allocator, class FileSystem::IFileSystem& fileSystem);

        virtual ~Server() { }

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

        const char* GetServerRole() const override
        {
            return Role == ServerRole::ReverseProxy ? "Reverse Proxy" : "Edge";
        }
        
    private:
        bool Init() override;
        uint16 Run() override;
        void Shutdown() override;

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

    public:
        /** Shared external server controls. */
        mutable CycleManager Controls;
        
    protected:
        SocketPooler Pooler;

        /** Address binder for Skylar server. */
        AddressBinder Binder;

        /** Modules. Server can act as a reverse proxy for another applications. */
        ModuleManager Modules;
        
        /** List of active sockets which listen for incoming connections. */
        TArray< Net::NetSocketBase* > Transports;
        
        Sync::Event QueueNotFullEvent;
        
        /** Global server mutex. */
        mutable Sync::SpinMutex Mutex;

        ServerRole Role;
    public:
        SkylarOptions Options;
        Net::SocketPool Sockets;

        friend class ConnectionDispatcher;
        friend class ModuleManager;
    private:

        IAllocator& Allocator;
        FileSystem::IFileSystem& FileSystem;

    private:
        NON_COPYABLE(Server)
    };
}

