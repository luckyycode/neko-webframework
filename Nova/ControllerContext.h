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

#include "Engine/Network/Http/Request.h"
#include "Engine/Network/Http/Response.h"
#include "Engine/Containers/Delegate.h"

#include "IControllerContext.h"

#include <functional>

namespace Neko
{
    namespace Nova
    {
        /** Controller context is created once on controller registration.
            It must only manage own controller lifetime. Contains controller info for actions, etc */
        template <class TController>
        struct ControllerContext final : public IControllerContext
        {
            // only controller interfaces
            static_assert(std::is_convertible<TController, IController>::value, "Must inherit from IController!");
            
            typedef std::function<class IController* (Http::Request&, Http::Response&) > CreateControllerFunc;
            
            /**
             *  Creates a new controller context for a controller type.
             */
            ControllerContext(IAllocator& allocator, Router& router, const char* path, const char* name)
            : Allocator(allocator)
            , Actions(allocator)
            , Router(router)
            {
                this->Path.Set(path);
                this->Path << "/";
                this->Path << name;
                
                this->Name.Set(name);
            }
            
            /**
             * Maps action to controller url.
             */
            template <void(TController::*A)()> ControllerContext& RouteAction(Http::Method method, const char* action, const char* params = nullptr)
            {
                static_assert(std::is_convertible<TController, IController>::value, "Route action class must inherit from IController!");
                
                if (IsValid())
                {
                    // Build controller action uri
                    StaticString<16> controllerActionPath(this->Path, "/", action, "/", params);
                    
                    // Save route
                    bool success = Router.AddRoute(method, *controllerActionPath, *this->Name, action);
                    if (success)
                    {
                        ControllerAction controllerAction;
                        controllerAction.Bind<TController, A>(nullptr);
                        
                        this->Actions.Insert(action, controllerAction);
                    }
                }
                else
                {
                    LogWarning.log("Nova") << "Couldn't add route for incorrect controller!";
                }
                
                return *this;
            }
            
            /** Clears this controller context information. */
            void Clear()
            {
                this->Actions.Clear();
                this->CreateControllerFunc = nullptr;
            }
            
            /**
             * Returns TRUE if controller information is valid (name, path).
             * If that information is not set then it's not a valid controller.
             */
            NEKO_FORCE_INLINE bool IsValid() const { return !this->Path.IsEmpty() && !this->Name.IsEmpty(); }
          
            virtual IController* CreateController(Http::Request& request, Http::Response& response) override
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
            Router& Router;
        };
    }
}
