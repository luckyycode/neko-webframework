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
//  ISocket.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/NekoString.h"
#include "Engine/Network/NetSocketBase.h"

namespace Neko::Skylar
{
    /** Wrapper for a system socket. */
    class ISocket
    {
    public:
        virtual ~ISocket() = default;
        
        /** @copydoc NetSocketBase::GetPacketBlocking */
        NEKO_FORCE_INLINE long GetPacketBlocking(TArray<String::value_type>& buffer, const int32& timeout) const
        {
            return GetPacketBlocking(&buffer[0], buffer.GetSize(), timeout);
        }
        
        /** @copydoc NetSocketBase::SendAllPacketsWait */
        NEKO_FORCE_INLINE long SendAllPacketsWait(const String& buffer, const int32& timeout) const
        {
            return SendAllPacketsWait(*buffer, buffer.Length(), timeout);
        }
        
        /** @copydoc NetSocketBase::GetPacketBlocking */
        virtual long GetPacketBlocking(void* buffer, const ulong length, const int32& timeout) const = 0;
        
        /** @copydoc NetSocketBase::SendAllPacketsWait */
        virtual long SendAllPacketsWait(const void* buffer, const ulong length, const int32& timeout) const = 0;
        
        /** @copydoc NetSocketBase::Close */
        virtual void Close() = 0;

        /** Returns the native socket handle this socket interface got bound to. */
        virtual Net::SocketHandle GetNativeHandle() const = 0;
        /** Tls session data. */
        virtual void* GetTlsSession() const = 0;
        
        // operators
        
        NEKO_FORCE_INLINE bool operator == (const ISocket& other) const
        {
            return this->GetNativeHandle() == other.GetNativeHandle();
        }
        
        NEKO_FORCE_INLINE bool operator != (const ISocket& other) const
        {
            return this->GetNativeHandle() != other.GetNativeHandle();
        }
    };
}


