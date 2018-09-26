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

#include "Engine/Utilities/NekoString.h"

namespace Neko
{
    namespace Nova
    {
        /// Transient routing info.
        class Routing
        {
        public:
            
            inline Routing(class IAllocator& allocator)
            : Parameters(allocator)
            , Controller(allocator)
            , Action(allocator)
            , IsValid(false)
            {
            }
            
            inline Routing(const String& controller, const String& action, const TArray<String>& parameters, bool valid = false)
            : Controller(controller)
            , Action(action)
            , Parameters(parameters)
            , IsValid(valid)
            {
            };
            
            inline void SetRouting(const String& controller, const String& action, const TArray<String>& parameters)
            {
                Controller = controller;
                Action = action;
                Parameters = parameters;
            };
            
            //! Says whether routing has valid values.
            bool IsValid;
            
            String Controller;
            String Action;
            
            // Incoming custom uri params.
            TArray<String> Parameters;
        };
    }
}
