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

#include "Router.h"

#include "ControllerContext.h"

namespace Neko
{
    namespace Http
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
            ControllerFactory(Router& router);
            
            /**
             * Prepares controller info.
             */
            template <class T> void CreateControllerContext(ControllerContext& context, const char* name, const char* path)
            {
                context.Path.Set(path);
                context.Name.Set(name);
                
                context.CreateController = [name, this] (Net::Http::Request& httpRequest, Net::Http::Response& httpResponse) -> IController*
                {
                    // create a new controller on request
                    return NEKO_NEW(Allocator, T) (httpRequest, httpResponse, Allocator, name);
                };
            }
            
            /**
             * Maps action to controller url.
             */
            template <typename T, void(T::*A)()> void RouteAction(ControllerContext& context, Net::Http::Method method, const char* action, const char* params = nullptr)
            {
                // Build controller#action string
                StaticString<32> controllerActionName(context.Name, "#", action);
                // Build controller action uri
                StaticString<32> controllerActionPath(context.Path, "/", action, "/", params);
                
                // Save route
                bool success = Router.AddRoute(method, *controllerActionPath, *controllerActionName);
                if (success)
                {
                    ControllerAction controllerAction;
                    controllerAction.Bind<T, A>(nullptr);
                    
                    context.Actions.Insert(action, controllerAction);
                }
            }
            
            /**
             * Saves the controller context.
             */
            void Save(ControllerContext& context);
            
            /**
             * Instantiates a new controller on request.
             */
            void ExecuteController(const Routing& routing, class IProtocol& protocol, Net::Http::Request& request, Net::Http::Response& response);
            
            /**
             * Removes controller data.
             */
            void ReleaseController(IController* controller);
            
        private:
            
            Router& Router;
            
            DefaultAllocator Allocator;
            
            THashMap<String, ControllerContext> ControllerDispatcher;
        };
    }
}
