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

#include "../../Engine/Core/Module.h"
#include "../../Engine/Data/IAllocator.h"
#include "../../Engine/Utilities/Tuple.h"
#include "../../Engine/Network/SocketQueue.h"
#include "../../Engine/Network/SocketList.h"

#include "Controls.h"
#include "../Settings.h"

#include "../Tls.h"

namespace Neko
{
    namespace Http
    {
        /// Server instance.
        class Server
        {
        public:
            
            Server(IAllocator& allocator, class FS::FileSystem& fileSystem);
            
            ~Server() { }
            
            /**
             * Binds socket port to the address.
             *
             * @param port  Port to bound address to.
             * @param ports Empty array of ports.
             * @return TRUE if succeeded, FALSE otherwise.
             */
            bool BindPort(const uint32 port, TArray<uint32>& ports);
            
            /** Prepares applications. E.g. binds found application ports and so on. */
            void PrepareApplications();
            
            /**
             * Manages worker threads.
             */
            int ProcessWorkerThreads(void* kek);
          
            /**
             * Processes incoming requests.
             */
            void ThreadRequestProc(class ISocket& socket, Net::SocketsQueue& sockets, void* stream) const;
            
            /** Initializes the server. */
            bool Init();
            
            /** Shuts down the server. */
            void Shutdown();
            
            int32 Run();
            
            /** Clears cached data and settings. */
            void Clear();
            
            /** Gets server process id (if multiple are running). */
            int GetServerProcessId(const String& serverName) const;
            
            /** Updates all server modules. */
            void UpdateModules();
            
            /** Updates the specified server module. */
            bool UpdateModule(Module& module, TArray< ApplicationSettings* >& applications, const uint32 moduleIndex);
            
        private:
            
            void* InitSsl(const ApplicationSettings& application);
            
            void CloseListeners();
            
        private:
            
            IAllocator& Allocator;
            
        public:
            
            void Stop();
            void Restart();
            void Update();
            
            int StartCommand(const String& name, bool force = false);
            int RestartCommand(const String& serverName) const;
            int ExitCommand(const String& serverName) const;
            int UpdateModulesCommand(const String& serverName) const;
            
        public:
            
            //! Shared external server controls.
            mutable ServerSharedControls Controls;
            
            void kek(void* kek);
            
        protected:
            
            SocketList SocketsList;
            
            //! Shared server settings
            ServerSettings Settings;
            
            //! Global server mutex.
            mutable MT::SpinMutex Mutex;
            
            MT::Event QueueNotFullEvent;
            
            //! Loaded server modules.
            TArray< Module > Modules;
            
            //! List of active sockets which listen for incoming connections.
            TArray< Net::INetSocket > Listeners;
            
            typedef THashMap<uint32, void*> TlsMap;
            //! Ssl contexts map.
            TlsMap TlsData;
            
        private:
            
            friend class RequestTask;
            
            //! Amount of active worker threads.
            mutable ThreadSafeCounter ThreadsWorkingCount;
        };
    }
}

