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
//  Http.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "IProtocol.h"
#include "../../Engine/Network/NetSocket.h"
#include "../../Engine/Network/Http/HttpStatusCodes.h"
#include "../../Engine/Network/Http/Request.h"

namespace Neko
{
    namespace Skylar
    {
        /// Http 1.1 capable server protocol.
        class ProtocolHttp final : public IProtocol
        {
        public:
            
            // See comments in IProtocol.
            
            ProtocolHttp(class ISocket& socket, const ServerSettings* settings, IAllocator& allocator);
            
            virtual IProtocol* Process() override;
            
            virtual long SendData(const void* source, ulong size, const int32& timeout, Http::DataCounter* dataCounter) const override;
            
            virtual bool SendHeaders(const Http::StatusCode status, TArray<std::pair<String, String> >& headers, const int32& timeout, bool end/* = true*/) const override;
            
            virtual void WriteRequest(char* data, const Http::Request& request, const ApplicationSettings& applicationSettings) const override;
            
            virtual void ReadResponse(Http::Request& request, const Http::ResponseData& responseData) const override;
            
            virtual void Close() override;
            
        private:
            
            const ApplicationSettings* GetApplicationSettingsForRequest(Http::Request& request, const bool secure) const;
            
            Http::StatusCode GetRequestData(Http::Request& request, String& buffer, const ApplicationSettings& applicationSettings) const;
            
        protected:
            
            /** Process a request using this protocol. */
            void RunProtocol(Http::Request& request, char* data, String& stringBuffer) const;
        };
    }
}

