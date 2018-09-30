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
//  SocketDefault.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "SocketDefault.h"

namespace Neko::Skylar
{
    SocketDefault::SocketDefault(const Net::INetSocket& socket)
        : Socket(socket)
    {
    }
    
    long SocketDefault::GetPacketBlocking(void* buffer, const ulong length, const int32& timeout) const
    {
        long size = 0;
        bool result = Socket.GetPacketBlocking(nullptr, buffer, size, length, timeout);
        NEKO_UNUSED(result);
        
        return size;
    }
    
    long SocketDefault::SendAllPacketsWait(const void* buffer, const ulong length, const int32& timeout) const
    {
        //Net::NetAddress address;
        //bool succeeded = Socket.GetAddress(address);
        
        return Socket.SendAllPacketsWait(nullptr, buffer, length, timeout);
    }
    
    void SocketDefault::Close()
    {
        // Send all data to client
        Socket.WaitForAnyDataInternal(-1, true);
        
        Socket.Close();
    }
}

