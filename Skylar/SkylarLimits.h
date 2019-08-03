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
//  SkylarLimits.h
//  Neko SDK
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/Date.h"

namespace Neko::Skylar
{
    struct PoolApplicationSettings;

    /** Skylar server limits. */
    struct SkylarLimits
    {
        uint64 MaxResponseBufferSize = 64 * 1024;
        uint64 MaxRequestBufferSize = 1024 * 1024;
        
        // Matches the default maxAllowedContentLength in IIS (~28.6 MB)
        // https://www.iis.net/configreference/system.webserver/security/requestfiltering/requestlimits#005
        uint64 MaxRequestBodySize = 30000000;
        
        // Matches the default LimitRequestFields in Apache httpd.
        int MaxRequestHeaderCount = 100;
        
        TimeSpan KeepAliveTimeout;
        
        ulong MaxConcurrentConnections = 0;
        
        SkylarLimits() : KeepAliveTimeout(0, 2, 0)
        { }
    };
}

