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
//  Cookie.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/Date.h"
#include "Engine/Utilities/NekoString.h"

namespace Neko::Nova
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
        { }
        
        inline Cookie()
        : Secure(false)
        , HttpOnly(false)
        { }
        
    public:
        /** Parses cookie header string to cookie list. */
        static bool ParseCookieString(const String& cookieString, TArray<Cookie>& outCookies);
        
        /** Returns TRUE if expiration date is NOT set. */
        NEKO_FORCE_INLINE bool IsSessionCookie() const { return this->ExpirationDate.IsValid() == false; }
        
        /** Converts this to string type. */
        String ToString(StringType type = StringType::Full);
        
    public:
        String Domain;
        String Path;
        String Comment;
        
        String Name;
        String Value;
        
        DateTime ExpirationDate;
        
        bool Secure;
        bool HttpOnly;
        
        bool SameSite;
        
        char pad[5];
    };
    
    using CookieJar = TArray<Cookie>;
}

