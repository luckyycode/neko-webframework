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
//  SessionStorageFactory.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "SessionStorageFactory.h"
#include "SessionCookieStorage.h"

#include "Engine/Utilities/NekoString.h"

#include "Session.h"

namespace Neko::Nova
{
    ISessionStorage* SessionStorageFactory::Get(const SessionStorageType type, IAllocator& allocator)
    {
        static const auto cookieType = SessionCookieStorage(allocator).GetType();
        
        ISessionStorage* result = nullptr;
        
        if (type == cookieType)
        {
            static SessionCookieStorage cookieStorage(allocator);
            result = &cookieStorage;
        }
        
        // we should always have something valid
        assert(result != nullptr);
        
        return result;
    }
    
    void SessionStorageFactory::Cleanup(const SessionStorageType type, ISessionStorage& storage)
    {
    }
}

