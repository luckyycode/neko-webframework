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
//  ListenOptions.h
//  Neko SDK
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/Array.h"
#include "ConnectionContext.h"
#include "ISsl.h"

namespace Neko::Skylar
{
    using MiddlewareDelegate = TDelegate<void(ConnectionContext&)>;
    
    /** Skylar server listening options. */
    struct ListenOptions
    {
        ListenOptions(IAllocator& allocator) : Middleware(allocator)
        { }
        
        const char* GetScheme() const
        {
            if (IsHttp)
            {
                return IsTls ? "https" : "http";
            }
            
            return "tcp";
        }
        
        /** Gets the name of this endpoint to display on command-line when the web server starts. */
        virtual String GetDisplayName() const
        {
            String displayName;
            displayName.Append(GetScheme())
                .Append("://unix:")
                .Append(*EndPoint.ToString());
            
            return displayName;
        }
        
        MiddlewareDelegate Build()
        {
            MiddlewareDelegate app;
            
            return app;
        }

        // Add the HTTP middleware as the terminal connection middleware
        void UseHttpServer()
        {
            IsHttp = true;
        }

        template <class TMiddleware>
        void Use()
        {
            auto& previousMiddleware = Middleware.back();
            TMiddleware middleware(previousMiddleware);
            Middleware.Push(middleware);
        }

        uint16 Port;
        
        bool IsHttp = true;
        bool IsTls;

        InitSslOptions SslOptions;

        Net::Endpoint EndPoint;
        
        TArray<MiddlewareDelegate> Middleware;
    };
}

