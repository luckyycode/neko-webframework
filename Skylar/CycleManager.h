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
//  CycleManager.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/Cache.h"
#include "Engine/Mt/Sync.h"
#include "Engine/Mt/ThreadSafe.h"
#include "Engine/Utilities/NonCopyable.h"

namespace Neko::Skylar
{
    /// Server threadsafe controls.
    class CycleManager
    {
    public:
        
        CycleManager()
        : ProcessQueueEvent(false)
        , UpdateModulesEvent(true)
        {
        }
        
        ~CycleManager()
        {
            Clear();
        }
        
        void Clear()
        {
            ProcessQueueEvent.Reset();
            UpdateModulesEvent.Reset();
        }
        
        void UpdateApplication()
        {
            UpdateModulesEvent.Trigger();
        }
        
        void ProcessQueue()
        {
            ProcessQueueEvent.Trigger();
        }
        
        void SetActiveFlag(const bool active = true)
        {
            Active = active;
        }
        
        void SetRestartFlag(const bool restart = true)
        {
            Restart = restart;
        }
        
        void StopProcess()
        {
            Active = false;
            
            ProcessQueue();
        }
        
    public:
        
        MT::Event ProcessQueueEvent;
        MT::Event UpdateModulesEvent;
        
        //! Says whether the server is active and has active threads.
        ThreadSafeBool Active;
        
        ThreadSafeBool Restart;
        
    private:
        
        NON_COPYABLE(CycleManager)
    };
}


