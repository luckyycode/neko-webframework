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
            , Allocator(allocator)
            , Controller(nullptr)
            {
            }
            
            /**
             * Prepares controller info.
             */
            template <class T> void Init(const char* name, const char* path)
            {
                static_assert(std::is_convertible<T, IController>::value, "Route action class must inherit IController!");
                
                this->Path.Set(path);
                this->Name.Set(name);
                
                this->CreateController = [name, this] (Net::Http::Request& httpRequest, Net::Http::Response& httpResponse) -> IController*
                {
                    // create a new controller on request
                    return NEKO_NEW(Allocator, T) (httpRequest, httpResponse, Allocator, name);
                };
            }
            
            /**
             * Maps action to controller url.
             */
            template <typename T, void(T::*A)()> void RouteAction(Router& router, Net::Http::Method method, const char* action, const char* params = nullptr)
            {
                static_assert(std::is_convertible<T, IController>::value, "Route action class must inherit IController!");
                
                // Build controller#action string
                StaticString<32> controllerActionName(this->Name, "#", action);
                // Build controller action uri
                StaticString<32> controllerActionPath(this->Path, "/", action, "/", params);
                
                // Save route
                bool success = router.AddRoute(method, *controllerActionPath, *controllerActionName);
                if (success)
                {
                    ControllerAction controllerAction;
                    controllerAction.Bind<T, A>(nullptr);
                    
                    this->Actions.Insert(action, controllerAction);
                }
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
            
            IAllocator& Allocator;
        };
    }
}
