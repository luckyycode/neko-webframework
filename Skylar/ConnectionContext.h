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
//  ConnectionContext.h
//  Neko SDK
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/Date.h"
#include "../Sockets/ISocket.h"

namespace Neko::Skylar
{
    /** Skylar server connection state. */
    struct ConnectionContext
    {
        ConnectionContext(Net::Endpoint& remoteEndpoint, int64 connectionId)
            : RemoteEndpoint(remoteEndpoint)
            , ConnectionId(connectionId)
        { }
        
        /** The unique identifier for the connection the request was received on. */
        int64 ConnectionId;
        
        //Net::Endpoint& LocalEndpoint;
        Net::Endpoint& RemoteEndpoint;
    };
    
    struct TransportConnection : ConnectionContext
    {
        TransportConnection(Net::Endpoint& remoteEndpoint, int64 connectionId)
            : ConnectionContext(remoteEndpoint, connectionId)
        { }
    };
    
    struct SkylarConnection
    {
        SkylarConnection(TransportConnection& connection, ISocket& socketConnection)
            : Connection(connection), SocketConnection(socketConnection)
        {
            
        }
        
        ISocket& SocketConnection;
        void* StreamData;
        
        TransportConnection& Connection;
    };
}

