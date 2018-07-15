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

#include "../../Engine/Utilities/NekoString.h"

#include "Session.h"

namespace Neko
{
    namespace Mvc
    {
        ISessionStorage* SessionStorageFactory::Get(const String& name)
        {
            static const String cookieName = SessionCookieStorage().GetName();
            
            ISessionStorage* result = nullptr;
            
            if (name == cookieName)
            {
                static SessionCookieStorage cookieStorage;
                result = &cookieStorage;
            }
            
            // we should always have something valid
            assert(result != nullptr);
            
            return result;
        }
        
        void SessionStorageFactory::Cleanup(const String& name, ISessionStorage& storage)
        {
        }
    
        TArray<String> SessionStorageFactory::GetAvailableStorageTypes()
        {
            TArray<String> result;
            result.Push(SessionCookieStorage().GetName().ToLower());
            
            return result;
        }
    }
}
