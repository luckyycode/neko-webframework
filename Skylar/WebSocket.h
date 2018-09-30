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
//  WebSocket.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "IProtocol.h"

namespace Neko::Skylar
{
    /// Websockets
    class ProtocolWebSocket final : public IProtocol
    {
    public:
        
        // @see comments in IProtocol
        
        ProtocolWebSocket(class ISocket& socket, const ServerSharedSettings* settings, class IAllocator& allocator);
        
        ProtocolWebSocket(const IProtocol& protocol);
        
        virtual bool    SendHeaders(const Http::StatusCode status, TArray< std::pair<String, String> >& headers, const int32& timeout, bool end) const override;
        virtual long    SendData(const void* source, ulong size, const int32& timeout, Http::DataCounter* dataCounter) const override;
        
        virtual void    WriteRequest(char* buffer, const Http::Request& repuest, const PoolApplicationSettings& applicationSettings) const override;
        virtual void    ReadResponse(Http::Request& request, const Http::ResponseData& responseData) const override;
        
        virtual IProtocol* Process() override;
        virtual void Close() override;
    };
}

