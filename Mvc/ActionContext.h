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
//  ActionContext.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "ControllerFactory.h"
#include "Router.h"

namespace Neko
{
	namespace Http
    {
        class ActionContext
        {
        public:
            
            ActionContext();
            
            int32 Execute(Net::Http::RequestData& requestData, Net::Http::ResponseData& responseData);
            
            void CleanupResponseData(void* responseData, uint32 responseSize);
            
        public:
            
            /**
             * Returns the instance of controller factory.
             */
            const ControllerFactory& GetControllerFactory() const
            {
                return this->ControllerFactory;
            }
            
        private:
            
            Router MainRouter;
            
            ControllerFactory ControllerFactory;
            
            Neko::DefaultAllocator Allocator;
        };
    }
}
