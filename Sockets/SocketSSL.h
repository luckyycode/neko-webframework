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
//  SocketSSL.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../Skylar/ISsl.h"
#include "ISocket.h"

#if USE_OPENSSL
#   include "Engine/Network/NetSocketBase.h"
#   include <openssl/ssl.h>

namespace Neko::Skylar
{
    /** OpenSSL socket wrapper */
    class SocketSSL : public ISocket
    {
    public:
        // @see ISocket for comments
        
        SocketSSL() = delete;
        /** Created a new SSL connection based on an existing context (server/client). */
        SocketSSL(const Net::NetSocketBase& socket, SSL_CTX& context);
        /** Initiates from an existing SSL connection. */
        SocketSSL(const Net::NetSocketBase& socket, SSL& connection);
        
        virtual long GetPacketAsync(void *buffer, const ulong length, const int32 &timeout) const override;
        virtual long SendAllPacketsAsync(const void *buffer, const ulong length, const int32 &timeout) const override;

        virtual void Close() override;
        
        virtual NEKO_FORCE_INLINE Net::SocketHandle GetNativeHandle() const override { return Socket.GetNativeHandle(); };
        virtual NEKO_FORCE_INLINE void* GetTlsSession() const override { return static_cast<void* >(this->Connection); };
        
        /** Higher level SSL connect */
        int32 Connect();
        bool Handshake();
        
    private:
        Net::NetSocketBase Socket;
        ::SSL* Connection;
    };
}
#endif

