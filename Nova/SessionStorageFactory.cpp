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
//  SessionStorageFactory.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "SessionStorageFactory.h"
#include "SessionCookieStorage.h"

#include "Engine/Utilities/NekoString.h"

#include "Session.h"

namespace Neko
{
    namespace Nova
    {
        ISessionStorage* SessionStorageFactory::Get(const SessionStorageType type)
        {
            static const auto cookieType = SessionCookieStorage().GetType();
            
            ISessionStorage* result = nullptr;
            
            if (type == cookieType)
            {
                static SessionCookieStorage cookieStorage;
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
}
