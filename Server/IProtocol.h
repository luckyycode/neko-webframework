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
//  IProtocol.h
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
        
        /// Interface capable for http, websockets, etc.
        class IProtocol
        {
        public:
            
            /**
             * Server protocol interface.
             *
             * @param socket    Currently active connection socket.
             * @param settings  Shared server settings.
             * @param controls  Shared server controls.
             */
            IProtocol(class ISocket& socket, const ServerSettings* settings, class IAllocator& allocator);
            
            IProtocol(const IProtocol& protocol);
            
            /** Virtual destructor. */
            virtual ~IProtocol() = default;
            
        public:
            
            /** Processes the request. */
            virtual IProtocol* Process() = 0;
            
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
            virtual void WriteRequestParameters(TArray<char>& buffer, const Net::Http::Request& request,
                                               const ApplicationSettings& applicationSettings) const = 0;
            
            /** Reads request buffer data. */
            virtual void ReadResponseParameters(Net::Http::Request& request, Net::Http::ResponseData& responseData) const = 0;
            
            /** Closes this protocol. */
            virtual void Close() = 0;
            
        private:
            
            /** Sends ready response headers. */
            bool SendHeaders(Net::Http::Response& response, const TArray<std::pair<String, String> >* extra, const uint32& timeout, const bool end = true) const;

            // shortcut
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
            void RunApplication(Net::Http::Request& request, const ApplicationSettings& applicationSettings) const;
            
        protected:
            
            //! Protocol socket instance.
            ISocket& Socket;
            
            //! This server settings.
            const ServerSettings* Settings;
            
            IAllocator& Allocator;
            
            //! Used for various timings
            CTimer Timer;
            
        public:
            
            /**
             * Sends a file. Can use partial send.
             *
             * @param request   Request containing outgoing information set (e.g. x-sendfile)
             */
            bool Sendfile(Net::Http::Request& request) const;
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

