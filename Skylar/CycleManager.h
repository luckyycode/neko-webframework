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
    /** Server threadsafe controls. */
    class CycleManager
    {
    public:
        CycleManager()
        : ProcessQueueSemaphore(0, INT_MAX)
        , UpdateModulesEvent(true)
        { }
        
        ~CycleManager()
        {
            Clear();
        }
        
        void Clear()
        {
            ProcessQueueSemaphore.Signal();
            UpdateModulesEvent.Reset();
        }
        
        void UpdateApplication()
        {
            UpdateModulesEvent.Notify();
        }
        
        void ProcessQueue()
        {
            ProcessQueueSemaphore.Signal();
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
        MT::Semaphore ProcessQueueSemaphore;
        MT::Event UpdateModulesEvent;
        
        //! Says whether the server is active and has active threads.
        MT::ThreadSafeBool Active;
        MT::ThreadSafeBool Restart;
        
    private:
        NON_COPYABLE(CycleManager)
    };
}


