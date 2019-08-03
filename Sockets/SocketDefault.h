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
//  SocketDefault.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "ISocket.h"

#include "Engine/Network/NetSocketBase.h"

namespace Neko::Skylar
{
    /**  Default network socket. Wrapper around Net::NetSocketBase. */
    class SocketDefault : public ISocket
    {
    public:
        // see ISocket for comments
        
        SocketDefault() = delete;
        SocketDefault(const Net::NetSocketBase& socket);
        
        virtual long GetPacketAsync(void *buffer, const ulong length, const int32 &timeout) const override;
        virtual long SendAllPacketsAsync(const void *buffer, const ulong length, const int32 &timeout) const override;
        
        virtual void Close() override;

        virtual NEKO_FORCE_INLINE Net::SocketHandle GetNativeHandle() const override { return Socket.GetNativeHandle(); };
        virtual NEKO_FORCE_INLINE void* GetTlsSession() const override { return nullptr; };
        
    private:
        Net::NetSocketBase Socket;
        
    };
}


