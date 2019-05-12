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
//  Router.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/Array.h"
#include "Engine/Utilities/NekoString.h"

#include "IRouter.h"

namespace Neko::Nova
{
    /** Routing object container. */
    class Router : public IRouter
    {
    public:
        Router(IAllocator& allocator)
        : Routes(allocator)
        , Allocator(allocator)
        { }
        
        virtual ~Router() { }
        
        bool AddRoute(const Http::Method method, const String& path, const String& controller, const String& action) override;
        
        Routing FindRoute(const String& method, const TArray<String>& components) const override;
        
        /** @copydoc Router::FindRoute */
        Routing FindRoute(Http::Method method, const TArray<String>& components) const;
        
        Routing FindRoute(Http::Method method, const String& uri) const override;
        
        String FindUrlByController(const String& controller, const String& action, const TArray<String>& params) const override;
        
        /** Splits the given path to array (differs from ParseIntoArray). */
        static void ParsePathForRoute(TArray<String>& outArray, const String& path);
        
    private:
        String GeneratePathFromComponents(const TArray<String>& components, const TArray<String>& params) const;
        
        void PrintAllRoutes();
        
    protected:
        void Clear();
        
    private:
        TArray<Route> Routes;
        
        IAllocator& Allocator;
    };
}

