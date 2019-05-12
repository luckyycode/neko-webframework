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
//  ISessionStorage.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Session.h"

namespace Neko
{
    namespace Nova
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
            virtual int32 ClearCache(const class DateTime& expiryDate) = 0;
        };
        
    }
}
