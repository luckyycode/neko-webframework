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
//  WebSocket.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Protocol.h"

namespace Neko::Skylar
{
    /** Websockets protocol */
    class ProtocolWebSocket final : public Protocol
    {
    public:
        // @see comments in Protocol
        ProtocolWebSocket(class ISocket& socket, const ProtocolOptions& options, class IAllocator& allocator);
        ProtocolWebSocket(const Protocol& protocol);
        
        virtual bool SendHeaders(const Http::StatusCode status, ListOfHeaderPair& headers, const int32& timeout, bool end) const override;
        virtual long SendData(const void* source, ulong size, const int32& timeout, Http::DataCounter* dataCounter) const override;
        
        virtual void WriteRequest(char* buffer, const Http::Request& repuest, const PoolApplicationSettings& applicationSettings) const override;
        virtual void ReadResponse(Http::Request& request, const Http::ResponseData& responseData) const override;
        
        virtual Protocol* Process() override;
        virtual void Close() override;
    };
}

