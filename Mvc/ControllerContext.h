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

#include "IControllerContext.h"

#include <functional>

namespace Neko
{
    namespace Mvc
    {
        /** Controller context is created once on initialization.
            It must only manage own controller lifetime. Contains controller info for actions, etc */
        template <class TController>
        struct ControllerContext : public IControllerContext
        {
            // only controller interfaces
            static_assert(std::is_convertible<TController, IController>::value, "Must inherit IController!");
            
            typedef std::function<class IController* (Net::Http::Request&, Net::Http::Response&) > CreateControllerFunc;
            
            /**
             *  Creates a new controller context for a controller type.
             */
            ControllerContext(IAllocator& allocator, const char* path, const char* name)
            : Allocator(allocator)
            , Actions(allocator)
            {
                this->Path.Set(path);
                this->Name.Set(name);
            }
            
            /**
             * Maps action to controller url.
             */
            template <void(TController::*A)()> ControllerContext& RouteAction(Router& router, Net::Http::Method method, const char* action, const char* params = nullptr)
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
                
                return *this;
            }
            
            void Clear()
            {
                this->Actions.Clear();
                this->CreateControllerFunc = nullptr;
            }
            
            virtual IController* CreateController(Net::Http::Request& request, Net::Http::Response& response) override
            {
                // create a new controller on request
                return NEKO_NEW(Allocator, TController) (request, response, Allocator);
            }
            
            virtual void ReleaseController(IController* controller) override
            {
                assert(controller != nullptr);
                NEKO_DELETE(Allocator, controller);
            }
            
            virtual void InvokeAction(IController& controller, const char* name) override
            {
                auto& controllerAction = this->Actions.at(name);
                assert(controllerAction.IsValid());
                
                controllerAction.InvokeWith(&controller);
            }
            
        private:
            
            //! Controller url path
            StaticString<16> Path;
            //! Controller name
            StaticString<16> Name;
            
            //! Controller action mappings
            THashMap<String, ControllerAction> Actions;
            
            IAllocator& Allocator;
        };
    }
}
