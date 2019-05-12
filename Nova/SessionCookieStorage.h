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
//  SessionCookieStorage.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/Date.h"
#include "Session.h"
#include "ISessionStorage.h"

namespace Neko
{
    namespace Nova
    {
        /// Cookie session storage.
        class SessionCookieStorage final : public ISessionStorage
        {
        public:
            SessionCookieStorage(IAllocator& allocator);

            // @see comments in ISessionStorage
            virtual Session Find(const String& sessionId) override;
            virtual int32 ClearCache(const DateTime& expireDate) override { return 0; };

            virtual bool Store(Session& session) override;
            virtual bool Remove(const String& sessionId) override { return true; };
            
            NEKO_FORCE_INLINE SessionStorageType GetType() const { return SessionStorageType::Cookie; }

        private:
            IAllocator& Allocator;
        };
    }
}
