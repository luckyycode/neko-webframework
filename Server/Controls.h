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
//  Controls.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../../Engine/Utilities/Cache.h"
#include "../../Engine/Mt/Sync.h"
#include "../../Engine/Mt/ThreadSafe.h"

namespace Neko
{
    namespace Http
    {
        /// Server threadsafe controls.
        class ServerSharedControls
        {
        public:
            
            ServerSharedControls()
            : ProcessQueueEvent(false)
            , QueueNotFullEvent(true)
            , UpdateModulesEvent(true)
            {
            }
            
            ~ServerSharedControls()
            {
                Clear();
            }
            
            void Clear()
            {
                QueueNotFullEvent.Reset();
                ProcessQueueEvent.Reset();
                UpdateModulesEvent.Reset();
            }
            
            void UpdateModule()
            {
                UpdateModulesEvent.Trigger();
            }
            
            void ProcessQueue()
            {
                ProcessQueueEvent.Trigger();
            }
            
            void SetActiveFlag(const bool set = true)
            {
                Active = set;
            }
            
            void SetRestartFlag(const bool set = true)
            {
                Restart = set;
            }
            
            void StopProcess()
            {
                Active = false;
                
                QueueNotFullEvent.Trigger();
                
                ProcessQueue();
            }
            
        public:
            
            MT::Event ProcessQueueEvent;
            MT::Event QueueNotFullEvent;
            MT::Event UpdateModulesEvent;
            
            //! Says whether the server is active and has active threads.
            ThreadSafeBool Active;
            
            ThreadSafeBool Restart;
        };
    }
}

