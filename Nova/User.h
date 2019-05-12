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
//  User.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/NekoString.h"

namespace Neko::Nova
{
    /** Interface class for user object. */
    class IUser
    {
    public:
        virtual ~IUser() { }
        
        /** Returns the identity key (i.e. user name) */
        virtual const String& GetPrimaryKey() const = 0;
        
        /** Returns the secondary key. */
        virtual const String& GetSecondaryKey() const { return String::Empty; }
    };
}

