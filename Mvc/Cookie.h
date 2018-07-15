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
//  Cookie.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../../Engine/Utilities/Date.h"
#include "../../Engine/Utilities/NekoString.h"

namespace Neko
{
    namespace Mvc
    {
        /// Session cookie.
        class Cookie
        {
        public:
            
            /** Used for convertation. */
            enum class StringType
            {
                NameValue = 0,
                
                Full
            };
            
            Cookie(const String& name, const String& value)
            : Name(name)
            , Value(value)
            {
                
            }
    
            inline Cookie()
            : Secure(false)
            , HttpOnly(false)
            {
                
            }
            
        public:
            
            /** Parses cookie header string to cookie list. */
            static bool ParseCookieString(const String& cookieString, TArray<Cookie>& outCookies);
            
            /** Returns TRUE if expiration date is NOT set. */
            NEKO_FORCE_INLINE bool IsSessionCookie() const { return !this->ExpirationDate.IsValid(); }
            
            /** Converts this to string type. */
            String ToString(StringType type = StringType::Full);
            
        public:
            
            CDateTime ExpirationDate;
            
            String Domain;
            String Path;
            String Comment;
            
            String Name;
            String Value;
            
            bool Secure : 1;
            bool HttpOnly : 1;
            
            // @todo SameSite
        };
        
        typedef TArray<Cookie> CookieJar;
    }
}
