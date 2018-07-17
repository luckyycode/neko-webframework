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
        class ProtocolHttp : public IProtocol
        {
        public:
            
            // See comments in IProtocol.
            
            ProtocolHttp(class ISocket& socket, const ServerSettings* settings, IAllocator& allocator);
            
            virtual IProtocol* Process() override;
            
            virtual long SendData(const void* source, uint32 size, const uint32& timeout, Net::Http::DataCounter* dataCounter) const override;
            
            virtual bool SendHeaders(const Net::Http::StatusCode status, TArray<std::pair<String, String> >& headers, const uint32& timeout, bool end/* = true*/) const override;
            
            virtual void WriteRequest(TArray<char>& data, const Net::Http::Request& request, const ApplicationSettings& applicationSettings) const override;
            
            virtual void ReadResponse(Net::Http::Request& request, Net::Http::ResponseData& responseData) const override;
            
            virtual void Close() override;
            
        private:
            
            const ApplicationSettings* GetApplicationSettingsForRequest(Net::Http::Request& request, const bool secure) const;
            
            Net::Http::StatusCode GetRequestData(Net::Http::Request& request, String& buffer, const ApplicationSettings& applicationSettings) const;
            
        protected:
            
            /** Process a request using this protocol. */
            void RunProtocol(Net::Http::Request& request, TArray<char>& data, String& stringBuffer) const;
        };
    }
}

