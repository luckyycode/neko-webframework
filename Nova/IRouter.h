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
//  IRouter.h
//  Neko SDK
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/Array.h"
#include "Engine/Utilities/NekoString.h"

#include "Route.h"
#include "Routing.h"

namespace Neko::Nova
{
    // Url router for controllers and actions.
    class IRouter
    {
    public:
        /**
         * Adds a new route to router.
         *
         * @param method    Http method (e.g. get, post, etc...)
         * @param path      Controller path.
         *
         * @return TRUE if route has been processed and successfully created.
         */
        virtual bool AddRoute(const Http::Method method, const String& path, const String& controller, const String& action) = 0;
        
        /**
         * Looks up for a routing.
         *
         * @param method        Http method as string.
         * @param components    Parsed request path.
         *
         * @return Routing object (check Valid field to check if routing is found).
         */
        virtual Routing FindRoute(const String& method, const TArray<String>& components) const = 0;
        virtual Routing FindRoute(Http::Method method, const String& uri) const = 0;
        
        /** Builds url with action and controller with parameters. */
        virtual String FindUrlByController(const String& controller, const String& action, const TArray<String>& params) const = 0;
    };
}

