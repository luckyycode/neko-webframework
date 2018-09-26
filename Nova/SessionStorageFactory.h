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
//  SessionStorageFactory.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/NekoString.h"
#include "Conventions/Enums/SessionCookieType.h"

namespace Neko
{
    namespace Nova
    {
        class ISessionStorage;
        
        /// Session storage factory. Creates the requested session storages.
        class SessionStorageFactory
        {
        public:
            
            /** Returns the list of available storage key type names. */
            //static TArray<SessionStorageFactory> GetAvailableStorageTypes();

            /** Lookups a needed storage by the key. */
            static ISessionStorage* Get(const SessionStorageType type);
            /** Removes data if session storage has created any. */
            static void Cleanup(const SessionStorageType type, ISessionStorage& storage);
        };
    }
}
