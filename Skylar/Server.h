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

#define DEFAULT_SERVER_NAME     "Skylar"

namespace Neko::Skylar
{
    /** Server instance. */
    class Server : public IServer
    {
    public:
        Server(IAllocator& allocator, class FileSystem::IFileSystem& fileSystem);
        
         virtual ~Server() { }
        
        /**
         * Binds socket port to the address.
         *
         * @param port  Port to bound address to.
         * @param ports Empty array of ports.
         * @return TRUE if succeeded, FALSE otherwise.
         */
        bool BindPort(const uint16 port, TArray<uint16>& ports);
        
        /** Prepares applications. Binds found application ports. */
        void PrepareApplications();
        
        /** Manages worker threads. */
        uint16 ProcessWorkerThreads();

        /** Initializes the server. */
        bool Init() override;
        
        /** Shuts down the server. */
        void Shutdown() override;
        
        uint16 Run() override;

        IAllocator& GetAllocator() override
        {
            return Allocator;
        }

        /** Clears cached data and settings. */
        void Clear();
        
        /** Gets server process id (if multiple are running). */
        uint32 GetServerProcessId(const String& serverName) const;

    private:
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
        //! Shared server settings
        ServerSharedSettings Settings;
        
    public:
        //! Shared external server controls.
        mutable CycleManager Controls;
        
    protected:
        SocketPooler SocketsList;
        
        /** List of active sockets which listen for incoming connections. */
        TArray< Net::NetSocketBase > Listeners;
        
        /** Loaded server modules. */
        TArray< Module > Modules;
        
        MT::Event QueueNotFullEvent;
        
        /** Global server mutex. */
        mutable MT::SpinMutex Mutex;

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

