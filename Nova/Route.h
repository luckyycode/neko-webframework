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
//  Route.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Prerequisites.h"

#include "Engine/Containers/Array.h"
#include "Engine/Utilities/NekoString.h"

namespace Neko::Nova
{
    /// Url route.
    struct Route
    {
    public:
        Route(IAllocator& allocator);
        
        TArray<String> Components;
        
        // url parameters (not query parameters)
        uint8 ParamCount;
        
        String Controller;
        String Action;
        
        TArray<int16>  ParamIndexes;
        bool HasCustomParams;
        
        Http::Method Method;
    };
}

