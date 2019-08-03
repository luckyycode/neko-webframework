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
//  SkylarOptions.h
//  Neko SDK
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#pragma once

#include "SkylarLimits.h"

namespace Neko::Skylar
{
    struct PoolApplicationSettings;

    /** Skylar server options. */
    struct SkylarOptions
    {
        TArray<ListenOptions> ListenOptions;

        bool AddServerHeader;
        
        bool AllowSynchronousIo;
        
        SkylarLimits Limits;
        
        SkylarOptions(IAllocator& allocator)
            : Limits()
            , ListenOptions(allocator)
        { }
    };
}

