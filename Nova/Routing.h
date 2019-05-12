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
//  Routing.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/NekoString.h"

namespace Neko::Nova
{
    /// Transient routing info.
    class Routing
    {
    public:
        inline Routing(class IAllocator& allocator)
            : Parameters(allocator), Controller(allocator)
            , Action(allocator), IsValid(false)
        { }
        
        inline Routing(const String& controller, const String& action, const TArray<String>& parameters,
                       bool valid = false)
            : Controller(controller), Action(action)
            , Parameters(parameters), IsValid(valid)
        { };
        
        inline void SetRouting(const String& controller, const String& action, const TArray<String>& parameters)
        {
            Controller = controller;
            Action = action;
            Parameters = parameters;
        };
        
        // Incoming custom uri params.
        TArray<String> Parameters;
        
        String Controller;
        String Action;
        
        //! Says whether routing has valid values.
        bool IsValid;
    };
}

