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
//  SessionStorageFactory.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/NekoString.h"
#include "Conventions/Enums/SessionCookieType.h"

namespace Neko::Nova
{
    class ISessionStorage;
    
    /** Session storage factory. Creates the requested session storages. */
    class SessionStorageFactory
    {
    public:
        /** Returns the list of available storage key type names. */
        //static TArray<SessionStorageFactory> GetAvailableStorageTypes();
        
        /** Lookups a needed storage by the key. */
        static ISessionStorage* Get(const SessionStorageType type, IAllocator& allocator);
        /** Removes data if session storage has created any. */
        static void Cleanup(const SessionStorageType type, ISessionStorage& storage);
    };
}

