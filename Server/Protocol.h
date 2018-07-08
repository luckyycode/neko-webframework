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
//  Protocol.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Controls.h"
#include "../Settings.h"

#include "../../Engine/Network/NetSocket.h"
#include "../../Engine/Network/Http/Request.h"
#include "../../Engine/Network/Http/Response.h"
#include "../../Engine/Network/Http/HttpStatusCodes.h"

#include "../../Engine/Network/Http/Extensions/Extensions.h"

#include "../../Engine/Containers/HashMap.h"
#include "../../Engine/Utilities/Timer.h"

#define DEFAULT_HTTPS_PORT  443
#define DEFAULT_HTTP_PORT   80

namespace Neko
{
    namespace Http
    {
        const static uint32 REQUEST_BUFFER_SIZE = 4096;
        
        /// Protocol interface capable for http, websockets, etc.
        class IServerProtocol
        {
        public:
            
            /**
             * Server protocol instance.
             *
             * @param socket    Currently active connection socket.
             * @param settings  Shared server settings.
             * @param controls  Shared server controls.
             */
            IServerProtocol(class ISocket& socket, const ServerSettings* settings, class IAllocator& allocator);
            
            IServerProtocol(const IServerProtocol& protocol);
            
            /** Virtual destructor. */
            virtual ~IServerProtocol() = default;
            
        public:
            
            /** Processes the request. */
            virtual IServerProtocol* Process() = 0;
            
            /**
             * Sends headers with status code.
             *
             * @param status    Http status code.
             * @param headers   Headers to send.
             * @param timeout   Timeout in msec.
             */
            virtual bool SendHeaders(const Net::Http::StatusCode status, TArray<std::pair<String, String> >& headers,
                                     const uint32& timeout, bool end = true) const = 0;
            
            /** Sends data by net socket. */
            virtual long SendData(const void* source, uint32 size,
                                  const uint32& timeout, Net::Http::DataCounter* dataCounter) const = 0;
            
            /** Writes request data to buffer. */
            virtual bool WriteRequestParameters(TArray<char>& buffer, const Net::Http::Request& request,
                                               const ServerApplicationSettings& applicationSettings) const = 0;
            
            /** Reads request buffer data. */
            virtual void ReadResponseParameters(Net::Http::Request& request, Net::Http::ResponseData& responseData) const = 0;
            
            /** Closes this protocol. */
            virtual void Close() = 0;
            
        private:
            
            bool SendHeaders(Net::Http::Response& response, const TArray<std::pair<String, String> >* extra, const uint32& timeout, const bool end = true) const;

            NEKO_FORCE_INLINE long SendData(const Net::Http::ObjectResult& data, const uint32& timeout) const
            {
                return SendData(data.Value, data.Size, timeout, nullptr);
            }
            
        public:
            
            /** Writes response. */
            bool SendResponse(Net::Http::Response& response, const uint32 timeout = Net::Http::DEFAULT_RESPONSE_TIME) const;
            
        public:
            
            /** Creates transient content data. Request content (type, length, params) */
            static ContentDesc* CreateContentDesc(const Net::Http::RequestDataInternal* requestData, const THashMap< String, IContentType* >& contentTypes, IAllocator& allocator);
            
            /** Destroys created content info description. */
            static void DestroyContentDesc(void* source, IAllocator& allocator);
            
        protected:
            
            /**
             * Runs application when request has been processed by protocol.
             *
             * @param request   Incoming request.
             * @param applicationSettings   Application.
             */
            void RunApplication(Net::Http::Request& request, const ServerApplicationSettings& applicationSettings) const;
            
        protected:
            
            //! Protocol socket instance.
            ISocket& Socket;
            
            //! This server settings.
            const ServerSettings* Settings;
            
            IAllocator& Allocator;
            
            //! Used for various timings
            CTimer Timer;
        };
        
        static NEKO_FORCE_INLINE const THashMap<int, const char*>& GetStatusList()
        {
            static DefaultAllocator allocator;
            static const THashMap<int, const char*> statusList(
            {
                // more to come...
                { 200, "OK" },
                { 206, "Partial Content" },
                { 304, "Not Modified" },
                { 400, "Bad Request" },
                { 401, "Unauthorized" },
                { 404, "Not Found" },
                { 413, "Request Entity Too Large" },
                { 416, "Requested Range Not Satisfiable" },
                { 500, "Internal Server Error" },
                { 501, "Not Implemented" },
                { 502, "Bad Gateway" },
            }, allocator);
            
            return statusList;
        }
        
    }
}

