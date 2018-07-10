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
//  Routing.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../../Engine/Utilities/NekoString.h"

namespace Neko
{
    namespace Mvc
    {
        /// Transient routing info.
        class Routing
        {
        public:
            
            inline Routing(class IAllocator& allocator)
            : Params(allocator)
            , Controller(allocator)
            , Action(allocator)
            , Valid(false)
            {
            }
            
            inline Routing(const String& controller, const String& action, const TArray<String>& params, bool valid = false)
            : Controller(controller)
            , Action(action)
            , Params(params)
            , Valid(valid)
            {
            };
            
            inline void SetRouting(const String& controller, const String& action, const TArray<String>& params)
            {
                Controller = controller;
                Action = action;
                Params = params;
            };
            
            //! Says whether routing has valid values.
            bool Valid;
            
            String Controller;
            String Action;
            
            // Incoming custom uri params.
            TArray<String> Params;
        };
    }
}
