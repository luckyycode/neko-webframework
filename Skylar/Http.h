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
//  Http.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Protocol.h"

#include "Engine/Network/Http/HttpStatusCodes.h"
#include "Engine/Network/Http/Request.h"
#include "Engine/Network/NetSocketBase.h"
#include "Engine/Utilities/NonCopyable.h"

namespace Neko::Skylar
{
    /** Http 1.1 capable server protocol. */
    class ProtocolHttp final : public Protocol
    {
    public:
        // See comments in Protocol.
        ProtocolHttp(class ISocket& socket, IAllocator& allocator);

        Protocol* Process() override;

        long SendData(const void* source, ulong size, const int32& timeout, Http::DataCounter* dataCounter) const override;
        bool SendHeaders(Http::StatusCode status, ListOfHeaderPair& headers, const int32& timeout,
                bool end/* = true*/) const override;
        
        void WriteRequest(char* data, const Http::Request& request,
                const PoolApplicationSettings& applicationSettings) const override;
        
        void ReadResponse(Http::Request& request, const Http::ResponseData& responseData) const override;
        void Close() override;
        
    private:
        const PoolApplicationSettings* GetApplicationSettingsForRequest(Http::Request& request, bool secure) const;

        Http::StatusCode GetRequestData(Http::Request& request, String& buffer,
                const PoolApplicationSettings& applicationSettings) const;
        
    protected:
        /** Process a request using this protocol. */
        void RunProtocol(Http::Request& request, char* data, String& stringBuffer) const;
        
    private:
        NON_COPYABLE(ProtocolHttp)
    };
}
