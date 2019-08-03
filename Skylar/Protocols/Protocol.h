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
//  Protocol.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../ModuleManager.h"

#include "Engine/Network/Http/Request.h"
#include "Engine/Network/Http/Response.h"
#include "Engine/Network/Http/HttpStatusCodes.h"

#include "Engine/Containers/HashMap.h"
#include "Engine/Utilities/Timer.h"
#include "../../Sockets/ISocket.h"
#include "../IProtocol.h"

#include "ProtocolOptions.h"

namespace Neko::Skylar
{
    using namespace Neko::Net;
    using namespace Neko::Net::Http;
    
    const static uint16 DefaultHttpsPort = 443;
    const static uint16 DefaultHttpPort = 80;
    
    const static uint32 RequestBufferSize = 4096;

    /** Base protocol for web-based requests. */
    class Protocol : public IProtocol
    {
    public:
        Protocol() = delete;

        /**
         * Server protocol interface.
         *
         * @param socket    Currently active connection socket.
         * @param settings  Shared server settings.
         * @param controls  Shared server controls.
         */
        Protocol(class ISocket& socket, const ProtocolOptions& options, class IAllocator& allocator);
        Protocol(const Protocol& protocol);

        /** Virtual destructor. */
        virtual ~Protocol() = default;
        
    public:
        /**
         * Sends headers with status code.
         *
         * @param status    Http status code.
         * @param headers   Headers to send.
         * @param timeout   Timeout in msec.
         */
        virtual bool SendHeaders(const Http::StatusCode status, ListOfHeaderPair& headers,
             const int32& timeout, bool end = true) const = 0;

        /** Writes request data to buffer. */
        virtual void WriteRequest(char* buffer, const Http::Request& request,
            const PoolApplicationSettings& applicationSettings) const = 0;
        
        /** Reads request buffer data. */
        virtual void ReadResponse(Http::Request& request, const Http::ResponseData& responseData) const = 0;

    private:
        /** Sends ready response headers. */
        bool SendHeaders(Http::Response& response, const ListOfHeaderPair* extra, const int32& timeout,
            const bool end = true) const;
        
        // shortcut
        NEKO_FORCE_INLINE long SendObjectData(const Http::ObjectResult &data, const uint32 &timeout) const
        {
            return IProtocol::SendData(data.Value, data.Size, timeout, nullptr);
        }
        
    public:
        /** Writes response. */
        bool SendResponse(Http::Response& response, const uint32 timeout = Http::DEFAULT_RESPONSE_TIME) const;
        
    public:
        /** Creates transient content data. Request content (type, length, params) */
        static ContentDesc* CreateContentDescriptor(const Http::RequestDataInternal& requestData,
            const THashMap< uint32, IContentType* >& contentTypes, IAllocator& allocator);
        
        /** Destroys created content info description. */
        static void DestroyContentDescriptor(void* source, IAllocator& allocator);
        
    protected:
        /**
         * Runs application when request has been processed by a protocol.
         *
         * @param request   Incoming request.
         * @param applicationSettings   Application.
         */
        void RunApplication(Http::Request& request, const PoolApplicationSettings& applicationSettings) const;
        
    protected:
        //! This server settings.
        const ModuleManager* Modules;
        const ProtocolOptions& Options;
        
        IAllocator& Allocator;
        
        //! Protocol socket instance.
        ISocket& Socket;
        
    public:
        void SetSettingsSource(const ModuleManager& moduleManager) override;
        
        /**
         * Sends a file. Can use partial send.
         *
         * @param request   Request containing outgoing information set (e.g. x-sendfile)
         */
        bool Sendfile(Http::Request& request) const;
    };

}
