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
//  ControllerFactory.c
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "../Server/IProtocol.h"

#include "ControllerFactory.h"
#include "IController.h"

namespace Neko
{
    namespace Http
    {
        ControllerFactory::ControllerFactory(class Router& router)
        : Allocator()
        , Router(router)
        , ControllerDispatcher(Allocator)
        {
        }
       
        void ControllerFactory::ExecuteController(const Routing& routing, IProtocol& protocol, Net::Http::Request& request, Net::Http::Response& response)
        {
            // get controller context
            auto contextIt = ControllerDispatcher.Find(*routing.Controller);
            if (contextIt.IsValid())
            {
                auto& context = contextIt.value();
                
                // create controller
                IController* controller = context.CreateController(request, response);
                
                assert(controller != nullptr);
                context.Controller = controller;
                
                // set params
                controller->SetUrlParameters(routing.Params);
                
                if (controller->PreFilter())
                {
                    // execute controller action
                    auto& controllerAction = context.Actions.at(*routing.Action);
                    controllerAction.InvokeWith(controller);
                    
                    controller->PostFilter();
                }
                
                // outgoing response is empty, probably controller didn't set anything
                if (response.Status != Net::Http::StatusCode::Empty)
                {
                    // but if something got changed..
                    protocol.SendResponse(response);
                }
                else
                {
                    // response has no data and empty status codes
                    if (!response.HasBodyData())
                    {
                        response.SetStatusCode(Net::Http::StatusCode::InternalServerError);
                    }
                }
                
                ReleaseController(controller);
                
                return;
            }
            
            // should never get there
            assert(false);
        }
        
        void ControllerFactory::ReleaseController(IController* controller)
        {
            assert(controller != nullptr);
            
            NEKO_DELETE(Allocator, controller);
        }
        
        void ControllerFactory::Save(ControllerContext& context)
        {
            String controllerName(context.Name);
            controllerName += "controller";
            ControllerDispatcher.Insert(controllerName, context);
        }
    }
}
