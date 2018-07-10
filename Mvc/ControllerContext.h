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
//  ControllerContext.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../../Engine/Network/Http/Request.h"
#include "../../Engine/Network/Http/Response.h"
#include "../../Engine/Containers/Delegate.h"

#include <functional>

namespace Neko
{
    namespace Mvc
    {
        /**
         * Points to the function of a controller.
         */
        typedef TDelegate< void() > ControllerAction;
        
        /** Contains controller info for actions, etc */
        struct ControllerContext
        {
            typedef std::function<class IController* (Net::Http::Request&, Net::Http::Response&) > CreateControllerFunc;
            
            ControllerContext(IAllocator& allocator)
            : Actions(allocator)
            , Controller(nullptr)
            {
            }
            
            void Clear()
            {
                assert(Controller == nullptr);
                this->Actions.Clear();
            }
            
            //! Controller url path
            StaticString<32> Path;
            //! Controller name
            StaticString<32> Name;
            
            //! Controller action mappings
            THashMap<String, ControllerAction> Actions;
            
            //! Controller create action.
            CreateControllerFunc CreateController;
            
            //! Transient controller info.
            IController* Controller;
        };
    }
}
