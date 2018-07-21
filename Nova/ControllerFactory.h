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

#include "../Skylar/IProtocol.h"
#include "Router.h"

#include "UserManager.h"
#include "SessionManager.h"
#include "ControllerContext.h"

namespace Neko
{
    namespace Nova
    {
        /// Provides managing of controllers lifetime.
        class ControllerFactory
        {
        public:
            
            ControllerFactory() = delete;
            
            /**
             * Constructor.
             *
             * @param router    Active url router.
             */
            ControllerFactory(Router& router, IAllocator& allocator);
      
            /**
             * Instantiates a new controller on request.
             *
             * @param routing   Route data.
             * @param protocol  Connection protocol.
             * @param request
             * @param response
             */
            void ExecuteController(const Routing& routing, Skylar::IProtocol& protocol, Http::Request& request, Http::Response& response);
            
            void Clear();
            
            /**
             * Creates a new controller context.
             *
             * @param controllerName    Controller name without controller postfix.
             * @param path              Route path to the controller (controller name will be appended automatically).
             *
             * @returns Valid controller context, if context failed to create then non-valid context will be returned.
             */
            template<class T>
            ControllerContext<T>& CreateContext(const char* controllerName, const char* pathToController)
            {
                ControllerContext<T>* context = NEKO_NEW(Allocator, ControllerContext<T>)(Allocator, Router, pathToController, controllerName);
                
                if (context == nullptr)
                {
                    LogError.log("Nova") << "Couldn't create a controller context.";
                    
                    static ControllerContext<T> dummyContext(Allocator, Router, "", "");
                    return dummyContext;
                }
                
                String fullControllerName(controllerName);
                fullControllerName += "controller";
                
                ControllerDispatcher.Insert(fullControllerName, static_cast< IControllerContext* >(context));
                
                return *context;
            }
            
        private:
            
            void SetSession(Http::Request& request, IController& controller);
            
        public:
            
            NEKO_FORCE_INLINE Router& GetRouter() { return this->Router; }
            
        private:
            
            Router& Router;
            
            UserManager UserManager;
            SessionManager SessionManager;
            
            IAllocator& Allocator;
            
            THashMap<String, IControllerContext* > ControllerDispatcher;
        };
    }
}
