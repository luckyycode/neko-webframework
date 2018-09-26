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
//  Http.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"

#define USE_OPENSSL 1

namespace Neko
{
    namespace Skylar
    {
        struct PoolApplicationSettings;
        
        /** Interface for ssl contexts. */
        class ISsl
        {
        public:
            
            typedef THashMap<uint16, void*> TlsMap;
            
            virtual bool Init() = 0;
            
            virtual void* InitSsl(const PoolApplicationSettings& application) = 0;
            virtual bool NegotiateProtocol(void* session, String& protocol) = 0;
            
            virtual void AddSession(uint16 port, void* context) = 0;
            virtual const TlsMap& GetTlsData() const = 0;
            
            virtual void Clear() = 0;
            
            static ISsl* Create(IAllocator& allocator);
            static void Destroy(ISsl& ssl);
        };
    }
}

