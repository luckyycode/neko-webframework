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
//  ConnectionManager.h
//  Neko SDK
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"
#include "ConnectionContext.h"

namespace Neko::Skylar
{
    // todo not used yet
    class ConnectionManager
    {
    public:
        ConnectionManager(IAllocator& allocator)
            : ConnectionReferences(allocator)
        { }
        
        void AddConnection(uint64 key, SkylarConnection& connection)
        {
            ConnectionReferences.Insert(key, connection);
        }
        
    private:
        THashMap<int64, SkylarConnection> ConnectionReferences;
    };
}

