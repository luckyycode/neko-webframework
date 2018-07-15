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
//  SessionManager.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Session.h"

namespace Neko
{
    namespace Mvc
    {
        /// Main class for managing sessions.
        class SessionManager
        {
        public:
            
            SessionManager() { }
            
            ~SessionManager() { }
            
            /** Gets a session by its id. */
            Session FindSession(const String& sessionId);
            
            /** Saves a session. */
            bool Store(Session& session);
            
            /** Removes a session. */
            bool Remove(const String& sessionId);
            
            
            /** Returns shared session storage type. */
            const String& GetStoreType() const;
            
            /** Clears session cache. */
            void ClearSessionsCache();
            
            
            /** Generates random & unique session id. */
            String GenerateSessionId();
            
            void SetCsrfProtectionData(Session& session);
            
            /** Clears a session and resets csrf data. */
            void ResetSession(Session& session);
        };
    }
}
