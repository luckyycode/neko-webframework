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
//  SocketDefault.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "ISocket.h"
#include "../Engine/Network/NetSocket.h"

namespace Neko
{
    namespace Skylar
    {
        /// Default network socket. Wrapper around Net::INetSocket.
        class SocketDefault : public ISocket
        {
        public:
            
            // see ISocket for comments
            
            SocketDefault() = delete;
            
            SocketDefault(const Net::INetSocket& socket);
            
            virtual long GetPacketBlocking(void* buffer, const uint32 length, const uint32& timeout) const override;
            
            virtual long SendAllPacketsWait(const void* buffer, const uint32 length, const uint32& timeout) const override;
            
            virtual void Close() override;
            
            
            virtual NEKO_FORCE_INLINE Net::SOCKET GetNativeHandle() const override { return Socket.GetNativeHandle(); };
            
            virtual NEKO_FORCE_INLINE void* GetTlsSession() const override { return nullptr; };
            
        private:
            
            Net::INetSocket Socket;
            
        };
    }
}

