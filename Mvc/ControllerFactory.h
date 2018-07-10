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
//  ControllerFactory.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../../Engine/Containers/Delegate.h"
#include "../../Engine/Utilities/StringUtil.h"

#include "../Server/IProtocol.h"
#include "Router.h"

#include "ControllerContext.h"

namespace Neko
{
    namespace Mvc
    {
        /// Provides managing controllers lifetime.
        class ControllerFactory
        {
        public:
            
            /**
             * Constructor.
             *
             * @param router    Active url router.
             */
            ControllerFactory(Router& router, IAllocator& allocator);
      
            /**
             * Saves the controller context.
             */
            void CreateControllerContext(ControllerContext& context);
            
            /**
             * Instantiates a new controller on request.
             */
            void ExecuteController(const Routing& routing, Http::IProtocol& protocol, Net::Http::Request& request, Net::Http::Response& response);
            
            /**
             * Removes controller data.
             */
            void ReleaseController(IController* controller);
            
        public:
            
            NEKO_FORCE_INLINE Router& GetRouter() { return this->Router; }
            
        private:
            
            Router& Router;
            
            IAllocator& Allocator;
            
            THashMap<String, ControllerContext> ControllerDispatcher;
        };
    }
}