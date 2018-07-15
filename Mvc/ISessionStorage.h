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
//  ISessionStorage.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Session.h"

#include "../../Engine/Utilities/Date.h"

namespace Neko
{
    namespace Mvc
    {
        /// Interface for different kinds of session storages (e.g. cookie, database, file..)
        class ISessionStorage
        {
        public:
            
            ISessionStorage()
            { }
            
            virtual ~ISessionStorage()
            { }
            
            /** Gets a session by the given id. */
            virtual Session Find(const String& sessionId) = 0;
            
            /** Saves a session. */
            virtual bool Store(Session& session) = 0;
            
            /** Removes session. */
            virtual bool Remove(const String& sessionId) = 0;
            
            /** Removes session cache. */
            virtual int32 ClearCache(const CDateTime& expiryDate) = 0;
        };
        
    }
}
