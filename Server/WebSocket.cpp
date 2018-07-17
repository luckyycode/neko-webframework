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
//  WebSocket.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "WebSocket.h"
#include "../ISocket.h"

namespace Neko
{
    namespace Skylar
    {
        // empty
        
        ProtocolWebSocket::ProtocolWebSocket(ISocket& socket, const ServerSettings* settings, IAllocator& allocator)
        : IProtocol(socket, settings, allocator)
        { }
        
        ProtocolWebSocket::ProtocolWebSocket(const IProtocol& protocol)
        : IProtocol(protocol)
        { }
        
        IProtocol* ProtocolWebSocket::Process()
        {
            return this;
        }
        
        long ProtocolWebSocket::SendData(const void* src,uint32 size, const uint32& timeout, Net::Http::DataCounter* dataCounter) const
        {
            return 0;
        }
        
        bool ProtocolWebSocket::SendHeaders(const Net::Http::StatusCode status, TArray< std::pair<String, String> >& headers, const uint32& timeout, bool end) const
        {
            return false;
        }
        
        void ProtocolWebSocket::WriteRequest(TArray<char>& buffer, const Net::Http::Request& repuest, const ApplicationSettings& applicationSettings) const
        {
        }
        
        void ProtocolWebSocket::ReadResponse(Net::Http::Request& request, Net::Http::ResponseData& responseData) const
        {
            
        }
        
        void ProtocolWebSocket::Close()
        {
            
        }
    }
}

