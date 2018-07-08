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
//  Router.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../../Engine/Containers/Array.h"
#include "../../Engine/Utilities/NekoString.h"

#include "Route.h"
#include "Routing.h"

namespace Neko
{
	namespace Http
    {
        // Url router for controllers.
        class Router
        {
        public:
            
            Router(IAllocator& allocator)
            : Routes(allocator)
            , Allocator(allocator)
            {
            }
            
            /**
             * Adds a new route to router.
             *
             * @param method    Http method (e.g. get, post, etc...)
             * @param path      Controller path.
             * @param controllerAction  Controller and action (format - controller#action).
             *
             * @return TRUE if route has been processed and successfully created.
             */
            bool AddRoute(const Net::Http::Method method, const String& path, const String& controllerAction);
            
            /**
             * Looks up for a routing.
             *
             * @param method        Http method as string.
             * @param components    Parsed request path.
             *
             * @return Routing object (check Valid field to check if routing is found).
             */
            Routing FindRouting(const String& method, TArray<String>& components) const;
            
            /** @copydoc Router::FindRouting */
            Routing FindRouting(Net::Http::Method method, TArray<String>& components) const;
            
            /** Builds url with action and controller with parameters. */
            String FindUrl(const String& controller, const String& action, const TArray<String>& params) const;
            
            /** Splits the given path to array (differs from ParseIntoArray). */
            static void SplitPath(TArray<String>& outArray, const String& path);
            
        private:
            
            String GeneratePath(const TArray<String>& components, const TArray<String>& params) const;
            
            void PrintAllRoutes();
            
        protected:
            
            void Clear();
            
        private:
            
            TArray<Route> Routes;
            
            IAllocator& Allocator;
        };
    }
}
