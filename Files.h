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
//  Files.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Server/IProtocol.h"
#include "../Engine/Network/Http/Request.h"

#include "../Engine/Utilities/Date.h"

namespace Neko
{
    namespace Http
    {
        /// x-sendfile support
        class SendfileExtension
        {
        public:
            
            /**
             * Sends a file. Can use partial send.
             *
             * @param protocol  Currently used server protocol (i.e. http/websocket)
             * @param request   Request containing outgoing information set (e.g. x-sendfile)
             * @param mimeTypes Supported mime types list.
             */
            static bool Send(const IProtocol& protocol, Net::Http::Request& request, const THashMap<String, String>& mimeTypes, IAllocator& allocator);
            
        private:
            
            /**
             * Sends a file. Can use partial send.
             *
             * @param protocol  Currently used server protocol (i.e. http/websocket)
             * @param request   Request containing outgoing information set (e.g. x-sendfile)
             * @param extraHeaders  Postprocessed headers
             * @param fileName  Requested file name.
             * @param mimeTypes Supported mime types list.
             * @param headersOnly   If TRUE then only header will be sent (with info such as 'Accept-Range' etc..)
             *
             * @return True if processing succeeded.
             */
            static bool Send(const IProtocol& protocol, const Net::Http::Request& request, TArray<std::pair<String, String> >& extraHeaders, const String& fileName, const THashMap<String, String>& mimeTypes, const bool headersOnly, IAllocator& allocator);
            
            // partial file send
            static bool SendPartial(const IProtocol& protocol, const Net::Http::Request& request, const String& fileName, CDateTime fileTime, const ulong fileSize, const String& rangeHeader, TArray<std::pair<String, String> >& extraHeaders, const THashMap<String, String>& mimeTypes, const bool headersOnly, IAllocator& allocator);
        };
    }
}

