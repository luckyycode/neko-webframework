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
//  SocketDefault.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "SocketDefault.h"

namespace Neko::Skylar
{
    SocketDefault::SocketDefault(const Net::NetSocketBase& socket)
        : Socket(socket)
    { }
    
    long SocketDefault::GetPacketAsync(void *buffer, const ulong length, const int32 &timeout) const
    {
        long size = 0;
        bool result = Socket.GetPacketAsync(nullptr, buffer, size, length, timeout);
        NEKO_UNUSED(result);
        
        return size;
    }
    
    long SocketDefault::SendAllPacketsAsync(const void *buffer, const ulong length, const int32 &timeout) const
    {
        //Net::Endpoint address;
        //bool succeeded = Socket.GetAddress(address);
        return Socket.SendAllPacketsAsync(nullptr, buffer, length, timeout);
    }
    
    void SocketDefault::Close()
    {
        // send all Data to a client
        Socket.WaitForAnyData(-1, true);
        Socket.Close();
    }
}

