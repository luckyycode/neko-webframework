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
//  IProtocol.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "IProtocol.h"
#include "../ContentTypes/ContentDesc.h"
#include "../ISocket.h"
#include "../Utils.h"

#include "../../Engine/Network/Http/Response.h"

#include "../../Engine/Core/Log.h"

namespace Neko
{
    namespace Http
    {
        IProtocol::IProtocol(ISocket& socket, const ServerSettings* settings, IAllocator& allocator)
        : Socket(socket)
        , Settings(settings)
        , Allocator(allocator)
        , Timer()
        { }
        
        IProtocol::IProtocol(const IProtocol& protocol)
        : Socket(protocol.Socket)
        , Settings(protocol.Settings)
        , Allocator(protocol.Allocator)
        , Timer()
        { }
   
        bool IProtocol::SendResponse(Net::Http::Response& response, const uint32 timeout) const
        {
            const auto& responseBody = response.GetBodyData();
            
            const uint8* data = responseBody.Value;
            const ulong size = responseBody.Size;
            
            assert(response.Status != Net::Http::StatusCode::Empty);
            
            bool result = SendHeaders(response, nullptr, timeout);
            // send data if have any
            if (result && data != nullptr && size > 0)
            {
                result = SendData(data, size, timeout, nullptr) > 0;
                
                Allocator.Deallocate((void* )data);
            }
            return result;
        }
        
        bool IProtocol::SendHeaders(Net::Http::Response& response, const TArray<std::pair<String, String> >* extra, const uint32& timeout, const bool end) const
        {
            TArray<std::pair<String, String> > headers(Allocator);
            
            const int32 size = response.Headers.GetSize() + (extra != nullptr ? extra->GetSize() : 0);
            
            headers.Reserve(size);
            
            for (auto iter = response.Headers.begin(), end = response.Headers.end(); iter != end; ++iter)
            {
                headers.Emplace(iter.key(), iter.value());
            }
            
            if (extra != nullptr)
            {
                for (auto& iter : *extra)
                {
                    headers.Emplace(iter.first, iter.second);
                }
            }
            
            return this->SendHeaders(response.Status, headers, timeout, end);
        }
        
        void IProtocol::RunApplication(Net::Http::Request& request, const ApplicationSettings& applicationSettings) const
        {
            TArray<char> buffer(Allocator);
            buffer.Reserve(REQUEST_BUFFER_SIZE);
            
            // write headers so application can read these
            if (!WriteRequestParameters(buffer, request, applicationSettings))
            {
                return;
            }
            
            // input
            Net::Http::RequestData requestData
            {
                Socket.GetNativeHandle(),
                Socket.GetTlsSession(),
                &buffer[0]
            };
            
            // output
            Net::Http::ResponseData responseData
            {
                nullptr, 0
            };
            
            assert(applicationSettings.OnApplicationRequest != nullptr);
            
            // Launch application
            request.ApplicationExitCode = applicationSettings.OnApplicationRequest(&requestData, &responseData);
            // check results
            if (request.ApplicationExitCode == APPLICATION_EXIT_SUCCESS)
            {
                if (responseData.Data != nullptr && responseData.Size > 0)
                {
                    ReadResponseParameters(request, responseData);
                    
                    // Clear application outgoing data
                    applicationSettings.OnApplicationPostRequest(&responseData);
                }
            }
            else
            {
                GLogWarning.log("Http") << "Application " << *applicationSettings.ServerModule << " exited with error.";
            }
        }
        
        ContentDesc* IProtocol::CreateContentDesc(const Net::Http::RequestDataInternal* requestData, const THashMap<String, IContentType* >& contentTypes, IAllocator& allocator)
        {
            auto it = requestData->IncomingHeaders.Find("content-type");
            
            if (!it.IsValid())
            {
                GLogInfo.log("Http") << "CreateContentDesc: No content-type header set.";
                return nullptr;
            }
            
            const String& headerValue = it.value();
            
            String contentTypeName(allocator);
            THashMap<String, String> contentParams(allocator);
            
            // Check if request content type has additional data
            int32 delimiter = headerValue.Find(";");
            
            // we have somethin
            if (delimiter != INDEX_NONE)
            {
                contentTypeName = headerValue.Mid(0, delimiter).Trim();
                
                for (int32 paramCur = delimiter + 1, paramEnd = 0; paramEnd != INDEX_NONE; paramCur = paramEnd + 1)
                {
                    paramEnd = headerValue.Find(";", ESearchCase::IgnoreCase, ESearchDir::FromStart, paramCur);
                    delimiter = headerValue.Find("=", ESearchCase::IgnoreCase, ESearchDir::FromStart, paramCur);
                    
                    if (delimiter >= paramEnd)
                    {
                        String paramName = headerValue.Mid(paramCur, (paramEnd != INDEX_NONE) ? paramEnd - paramCur : INT_MAX).Trim();
                        
                        contentParams.Insert(Neko::Move(paramName), Neko::String(allocator));
                    }
                    else
                    {
                        String paramName = headerValue.Mid(paramCur, delimiter - paramCur).Trim();
                        
                        ++delimiter;
                        
                        String paramValue = headerValue.Mid(delimiter, (paramEnd != INDEX_NONE) ? paramEnd - delimiter : INT_MAX).Trim();
                    
                        contentParams.Insert(Neko::Move(paramName), Neko::Move(paramValue)
                                             );
                    }
                }
            }
            else
            {
                contentTypeName = headerValue;
            }
            
            const auto contentTypeIt = contentTypes.Find(contentTypeName);
            
            if (!contentTypeIt.IsValid())
            {
                return nullptr;
            }
            
            ulong dataLength = 0;
            
            const IContentType* contentType = contentTypeIt.value();
            const auto contentLengthIt = requestData->IncomingHeaders.Find("content-length");
            
            if (contentLengthIt.IsValid())
            {
                dataLength = StringToUnsignedLong(*contentLengthIt.value());
            }
            
            // transient content data
            void* state = contentType->CreateState(requestData, contentParams);
            
            ContentDesc* contentDesc = NEKO_NEW(allocator, ContentDesc)
            {
                dataLength,
                0, 0,
                state,
                nullptr,
                contentType,
            };
            
            return contentDesc;
        }
        
        void IProtocol::DestroyContentDesc(void* source, IAllocator& allocator)
        {
            ContentDesc* content = (ContentDesc* )source;
            
            if (content != nullptr)
            {
                content->ContentType->DestroyState(content->State);
                
                if (content->Data)
                {
                    NEKO_DELETE(allocator, (String* )content->Data) ;
                    content->Data = nullptr;
                }
            }
            
            NEKO_DELETE(allocator, content) ;
        }
    }
}

