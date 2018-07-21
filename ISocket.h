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
//  ISocket.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../Engine/Utilities/NekoString.h"
#include "../Engine/Network/NetSocket.h"

#define USE_OPENSSL 1

namespace Neko
{
    namespace Skylar
    {
        /** Wrapper for system socket. */
        class ISocket
        {
        public:
            
            virtual ~ISocket() = default;
            
            /** @copydoc INetSocket::GetPacketBlocking */
            NEKO_FORCE_INLINE long GetPacketBlocking(TArray<String::value_type>& buffer, const int32& timeout) const
            {
                return GetPacketBlocking(&buffer[0], buffer.GetSize(), timeout);
            }
            
            /** @copydoc INetSocket::SendAllPacketsWait */
            NEKO_FORCE_INLINE long SendAllPacketsWait(const String& buffer, const int32& timeout) const
            {
                return SendAllPacketsWait(*buffer, buffer.Length(), timeout);
            }
            
            /** @copydoc INetSocket::GetPacketBlocking */
            virtual long GetPacketBlocking(void* buffer, const ulong length, const int32& timeout) const = 0;
            
            /** @copydoc INetSocket::SendAllPacketsWait */
            virtual long SendAllPacketsWait(const void* buffer, const ulong length, const int32& timeout) const = 0;
            
            /** @copydoc INetSocket::Close */
            virtual void Close() = 0;
            
            
            /** Returns the native socket handle this socket interface got bound to. */
            virtual Net::SOCKET GetNativeHandle() const = 0;
            
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
}

