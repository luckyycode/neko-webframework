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
//  SocketSSL.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../Skylar/ISsl.h"
#include "ISocket.h"

#if USE_OPENSSL

#   include "Engine/Network/NetSocket.h"

#   include <openssl/ssl.h>

namespace Neko
{
    namespace Skylar
    {
        /** OpenSSL socket wrapper */
        class SocketSSL : public ISocket
        {
        public:
            
            // @see ISocket for comments
            
            SocketSSL() = delete;
            /** Created a new SSL connection based on an existing context (server/client). */
            SocketSSL(const Net::INetSocket& socket, SSL_CTX* context);
            /** Initiates from an existing SSL connection. */
			SocketSSL(const Net::INetSocket& socket, SSL* connection);
			
            virtual long GetPacketBlocking(void* buffer, const ulong length, const int32& timeout) const override;
            
            virtual long SendAllPacketsWait(const void* buffer, const ulong length, const int32& timeout) const override;
            
            virtual void Close() override;
            
            
            virtual NEKO_FORCE_INLINE Net::SOCKET GetNativeHandle() const override { return Socket.GetNativeHandle(); };
            
            virtual NEKO_FORCE_INLINE void* GetTlsSession() const override { return static_cast<void* >(this->Connection); };
            
            /** Higher level SSL connect */
            int32 Connect();
            
			bool Handshake();
			
        private:
            
            Net::INetSocket Socket;
            
            SSL* Connection;
        };
    }
}

#endif
