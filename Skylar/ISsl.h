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
//  Http.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"

#define USE_OPENSSL 1

namespace Neko::Skylar
{
    struct PoolApplicationSettings;

    /** Interface for the ssl contexts. */
    class ISsl
    {
    public:
        using TlsMap = THashMap<uint16, void* >;

        /** Initializes the system. */
        virtual bool Init() = 0;

        /** Initializes ssl session for the application. */
        virtual void* InitSslFor(const PoolApplicationSettings& application) = 0;

        /** Tries to negotiate a protocol. Outputs the protocol name. */
        virtual bool NegotiateProtocol(void* context, char* protocol) = 0;

        /** Adds valid context/session for the bound port. */
        virtual void AddSession(uint16 port, void* context) = 0;
        virtual const TlsMap& GetTlsData() const = 0;

        /** Clears all sessions. */
        virtual void Clear() = 0;
        
    public:
        static ISsl* Create(IAllocator& allocator);
        static void Destroy(ISsl& ssl);
    };
}

