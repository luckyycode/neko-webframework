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

#include "IController.h"

#include <functional>

namespace Neko
{
    namespace Mvc
    {
        /**
         * Points to the function of a controller.
         */
        typedef TDelegate< void() > ControllerAction;
        
        struct IControllerContext
        {
            virtual IController* CreateController(Net::Http::Request& request, Net::Http::Response& response) = 0;
            
            virtual ControllerAction& GetAction(const char* name) = 0;
        };
        
        /** Contains controller info for actions, etc */
        template <class TController>
        struct ControllerContext : public IControllerContext
        {
            static_assert(std::is_convertible<TController, IController>::value, "Must inherit IController!");
            
            typedef std::function<class IController* (Net::Http::Request&, Net::Http::Response&) > CreateControllerFunc;
            
            ControllerContext(IAllocator& allocator)
            : Controller(nullptr)
            , Allocator(allocator)
            , Actions(allocator)
            {
            }
            
            /**
             * Prepares controller info.
             */
            void Init(const char* name, const char* path)
            {
                this->Path.Set(path);
                this->Name.Set(name);
                
                this->CreateIController = [name, this] (Net::Http::Request& httpRequest, Net::Http::Response& httpResponse) -> IController*
                {
                    // create a new controller on request
                    return NEKO_NEW(Allocator, TController) (httpRequest, httpResponse, Allocator, name);
                };
            }
            
            /**
             * Maps action to controller url.
             */
            template <void(TController::*A)()> void RouteAction(Router& router, Net::Http::Method method, const char* action, const char* params = nullptr)
            {
                static_assert(std::is_convertible<TController, IController>::value, "Route action class must inherit IController!");
                
                // Build controller#action string
                StaticString<32> controllerActionName(this->Name, "#", action);
                // Build controller action uri
                StaticString<32> controllerActionPath(this->Path, "/", action, "/", params);
                
                // Save route
                bool success = router.AddRoute(method, *controllerActionPath, *controllerActionName);
                if (success)
                {
                    ControllerAction controllerAction;
                    controllerAction.Bind<TController, A>(nullptr);
                    
                    this->Actions.Insert(action, controllerAction);
                }
            }
            
            void Clear()
            {
                assert(Controller == nullptr);
                
                this->Actions.Clear();
                this->CreateControllerFunc = nullptr;
            }
            
            virtual IController* CreateController(Net::Http::Request& request, Net::Http::Response& response) override
            {
                this->Controller = static_cast<TController* >(this->CreateIController(request, response));
                return this->Controller;
            }
            
            virtual ControllerAction& GetAction(const char* name) override
            {
                return this->Actions.at(name);
            }
            
            //! Controller url path
            StaticString<16> Path;
            //! Controller name
            StaticString<16> Name;
            
            //! Controller action mappings
            THashMap<String, ControllerAction> Actions;
            
            //! Controller create action.
            CreateControllerFunc CreateIController;
            
            IAllocator& Allocator;
            
            //! Transient controller info.
            TController* Controller;
        };
    }
}
