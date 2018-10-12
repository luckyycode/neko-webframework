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
//  Server.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Core/Module.h"
#include "Engine/Data/IAllocator.h"
#include "Engine/Utilities/Tuple.h"
#include "Engine/Network/SocketQueue.h"
#include "Engine/Platform/SocketList.h"

#include "CycleManager.h"
#include "../SharedSettings.h"

#include "../Tls.h"

#define DEFAULT_SERVER_NAME     "Skylar"

namespace Neko::Skylar
{
    /** Server instance. */
    class Server
    {
    public:
        
        Server(IAllocator& allocator, class FileSystem::IFileSystem& fileSystem);
        
        ~Server() { }
        
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
        
        /**
         * Manages worker threads.
         */
        uint16 ProcessWorkerThreads(Server* instance, Net::SocketsQueue& sockets);
      
        /**
         * Processes incoming requests.
         */
        void ThreadRequestProc(class ISocket& socket, Net::SocketsQueue& sockets, void* stream) const;
        
        /** Initializes the server. */
        bool Init();
        
        /** Shuts down the server. */
        void Shutdown();
        
        uint16 Run();
        
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
        
    private:
        
        IAllocator& Allocator;
        
        FileSystem::IFileSystem& FileSystem;
        
    public:
        
        // commands
        
        void Stop();
        void Restart();
        void Update();
        
        uint16 StartCommand(const String& name, bool force = false);
        uint16 RestartCommand(const String& serverName) const;
        uint16 ExitCommand(const String& serverName) const;
        uint16 UpdateModulesCommand(const String& serverName) const;
        
    public:
        
        //! Shared external server controls.
        mutable CycleManager Controls;
        
        void kek(void* kek);
        
    protected:
        
        SocketList SocketsList;
        
        //! Shared server settings
        ServerSharedSettings Settings;
        
        //! Global server mutex.
        mutable MT::SpinMutex Mutex;
        
        MT::Event QueueNotFullEvent;
        
        //! Loaded server modules.
        TArray< Module > Modules;
        
        //! List of active sockets which listen for incoming connections.
        TArray< Net::INetSocket > Listeners;
        
        class ISsl* Ssl;
        
    private:
        
        friend class RequestTask;
        
        //! Amount of active worker threads.
        mutable ThreadSafeCounter ThreadsWorkingCount;
        
    private:
        
        NON_COPYABLE(Server)
    };
}

