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
//  Cookie.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Cookie.h"

#include "Engine/Utilities/Templates.h"
#include "Engine/Utilities/Utilities.h"

namespace Neko::Nova
{
    bool Cookie::ParseCookieString(const String& cookieHeader, TArray<Cookie>& outCookies)
    {
        if (cookieHeader.IsEmpty())
        {
            return false;
        }

        for (int32 current = 0, next; current != INDEX_NONE; current = next )
        {
            next = cookieHeader.Find(";", current);
            int32 c = next == INDEX_NONE ? INT_MAX : next;

            int32 delimiter = cookieHeader.Find("=", current);

            if (delimiter == INDEX_NONE or delimiter > c)
            {
                return false;
            }

            String key = cookieHeader.Mid(current, delimiter - current).Trim();
            String cookieKey;

            Util::DecodeUrl(key, cookieKey);

            ++delimiter;

            String value = cookieHeader.Mid(delimiter, next != INDEX_NONE ? next - delimiter : INT_MAX).Trim();
            String cookieValue;

            Util::DecodeUrl(value, cookieValue);

            outCookies.Emplace(Neko::Move(cookieKey), Neko::Move(cookieValue));

            if (next != INDEX_NONE)
            {
                ++next;
            }
        }

        return true;
    }

    String Cookie::ToString(StringType type/* = StringType::Full*/)
    {
        String result;

        if (Name.IsEmpty())
        {
            return result;
        }

        result = Name;
        result += "=";
        result += Value;

        // lets build some plain string
        if (type == StringType::Full)
        {
            if (Secure) { result += "; secure"; }
            if (HttpOnly) { result += "; httpOnly"; }
            if (SameSite) { result += "; samesite"; }

            if (not IsSessionCookie())
            {
                result += "; expires=";
                result += ExpirationDate.ToRfc882();
            }

            if (not Domain.IsEmpty())
            {
                result += "; domain=";
                result += Domain;
            }

            if (not Path.IsEmpty())
            {
                result += "; path=";
                result += Path;
            }
        }
        return result;
    }
}
