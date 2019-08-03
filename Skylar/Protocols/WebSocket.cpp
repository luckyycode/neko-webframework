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
//  WebSocket.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "WebSocket.h"
#include "../../Sockets/ISocket.h"

namespace Neko::Skylar
{
    // empty
    ProtocolWebSocket::ProtocolWebSocket(ISocket& socket, const ProtocolOptions& options, IAllocator& allocator)
    : Protocol(socket, options, allocator)
    { }
    
    ProtocolWebSocket::ProtocolWebSocket(const Protocol& protocol)
    : Protocol(protocol)
    { }
    
    Protocol* ProtocolWebSocket::Process()
    {
        return this;
    }
    
    long ProtocolWebSocket::SendData(const void* src,ulong size, const int32& timeout, Http::DataCounter* dataCounter) const
    {
        return 0;
    }
    
    bool ProtocolWebSocket::SendHeaders(const Http::StatusCode status, ListOfHeaderPair& headers, const int32& timeout, bool end) const
    {
        return false;
    }
    
    void ProtocolWebSocket::WriteRequest(char* buffer, const Http::Request& repuest, const PoolApplicationSettings& applicationSettings) const
    {
    }
    
    void ProtocolWebSocket::ReadResponse(Http::Request& request, const Http::ResponseData& responseData) const
    {
    }
    
    void ProtocolWebSocket::Close()
    {
    }
}


