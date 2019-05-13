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

namespace Neko::Nova
{
    /** Controller context is created once on controller registration.
     It must only manage own controller lifetime. Contains controller info for actions, etc */
    template <class TController>
    struct ControllerContext final : public IControllerContext
    {
        // only controller interfaces
        static_assert(std::is_convertible<TController, IController>::value, "Must inherit from IController!");
        
        using CreateControllerFunc = std::function<class IController* (Http::Request&, Http::Response&) >;
        using ActionMap = THashMap<uint32, ControllerAction>;

        /**  Creates a new controller context for a controller type. */
        ControllerContext(IAllocator& allocator, IRouter& router, const char* path, const char* name)
            : Allocator(allocator)
            , Actions(allocator)
            , Router(router)
            , Path(path)
            , Name(name)
        {
            Path << "/" << name;
        }

        /** Maps action to controller url. */
        template <void(TController::*A)()> ControllerContext& RouteAction(Http::Method method, const char* action,
            const char* params = nullptr)
        {
            static_assert(std::is_convertible<TController, IController>::value,
                          "Route action class must inherit from IController!");
            
            if (IsValid())
            {
                // Build controller action uri
                StaticString<32> controllerActionPath(this->Path, "/", action, "/", params);
                
                // Save route
                if (Router.AddRoute(method, *controllerActionPath, *this->Name, action))
                {
                    ControllerAction controllerAction;
                    controllerAction.Bind<TController, A>(nullptr);
                    
                    const uint32 actionHash = Crc32(action);
                    this->Actions.Insert(actionHash, controllerAction);
                }
            }
            else
            {
                LogWarning.log("Nova") << "Couldn't add route for an incorrect controller!";
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
        NEKO_FORCE_INLINE bool IsValid() const { return not this->Path.IsEmpty() && not this->Name.IsEmpty(); }
        
        IController* CreateController(Http::Request& request, Http::Response& response) override
        {
            // create a new controller on request
            return NEKO_NEW(Allocator, TController) (request, response, Allocator);
        }
        
        void ReleaseController(IController& controller) override
        {
            NEKO_DELETE(Allocator, &controller);
        }
        
        void InvokeAction(IController& controller, const String& name) override
        {
            const uint32 actionHash = Crc32(*name);
            
            auto& controllerAction = this->Actions.at(actionHash);
            assert(controllerAction.IsValid());
            
            controllerAction.InvokeWith(&controller);
        }
        
    private:
        /** Controller action mappings */
        ActionMap Actions;

        /** Router instance containing all needed paths. */
        IRouter& Router;
        IAllocator& Allocator;
        
        /** Controller url path */
        StaticString<32> Path;
        /** Controller name */
        StaticString<32> Name;
    };
}

