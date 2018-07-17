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
//  SessionCookieStorage.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Session.h"
#include "ISessionStorage.h"

namespace Neko
{
    namespace Nova
    {
        /// Cookie session storage.
        class SessionCookieStorage : public ISessionStorage
        {
        public:
            
            // @see comments in ISessionStorage
            
            virtual Session Find(const String& sessionId) override;
            
            virtual int32   ClearCache(const DateTime& expireDate) override { return 0; };
            
            virtual bool Store(Session& session) override;
            
            virtual bool Remove(const String& sessionId) override { return true; };
            
            NEKO_FORCE_INLINE String GetName() const { return "cookie"; }
        };
    }
}
