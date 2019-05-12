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
//  IControllerContext.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Network/Http/Request.h"
#include "Engine/Network/Http/Response.h"

#include "IController.h"

namespace Neko::Nova
{
    /** Points to a function of a controller. */
    using ControllerAction = TDelegate< void() >;

    /** Controller context interface. */
    struct IControllerContext
    {
        /** Virtual destructor. */
        virtual ~IControllerContext() { }

        /**
         * Creates a new instance of controller.
         *
         * @param request   Http request. Request must be never destroyed early than this controller instance.
         * @param response  Http response. Response must be never destroyed early than this controller instance.
         *
         * @returns Instance of a new controller.
         */
        virtual IController* CreateController(Http::Request& request, Http::Response& response) = 0;

        /** Destroys the given controller. */
        virtual void ReleaseController(IController& controller) = 0;

        /**
         * Invokes the controller registered action.
         *
         * @param controller    Action controller.
         * @param name          A name of the action to execute.
         */
        virtual void InvokeAction(IController& controller, const String& name) = 0;
    };
}
