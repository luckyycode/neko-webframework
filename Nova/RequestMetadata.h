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
//  RequestContext.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Core/Path.h"

namespace Neko::Nova
{
    struct RequestMetadata
    {
        void Deserialize(class Neko::InputData& data);
        
        // incoming protocol type by request
        class Protocol* Protocol = nullptr;
        class ISocket* Socket = nullptr;
        
        bool Secure;
        
        // application document root
        char DocumentRoot[Neko::MAX_PATH_LENGTH];
    };
}

