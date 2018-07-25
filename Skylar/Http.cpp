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
//  Http.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Http.h"
#include "WebSocket.h"

#include "Engine/Core/Profiler.h"
#include "Engine/Core/Log.h"
#include "Engine/Data/Blob.h"
#include "Engine/Utilities/NekoString.h"
#include "Engine/Utilities/Cache.h"
#include "Engine/Platform/Platform.h"

#include "../ContentTypes/ContentDesc.h"
#include "../Sockets/ISocket.h"
#include "../Utils.h"

// Http 1.1 capable protocol

namespace Neko
{
    namespace Skylar
    {
        ProtocolHttp::ProtocolHttp(ISocket& socket, const ServerSettings* settings, IAllocator& allocator)
        : IProtocol(socket, settings, allocator)
        {
            
        }

        void ProtocolHttp::WriteRequest(char* buffer, const Http::Request& request, const ApplicationSettings& applicationSettings) const
        {
            OutputData blob(reinterpret_cast<void* >(buffer), INT_MAX);
            
            // version
            blob.Write(uint8(Http::Version::Http_1));
            
            // request data
            blob << request.Host << request.Path << request.Method;
            
            // app
            blob.WriteString(applicationSettings.RootDirectory);
            
            // containers
            blob << request.IncomingHeaders << request.IncomingData << request.IncomingFiles /* files */;
        }
        
        void ProtocolHttp::ReadResponse(Http::Request& request, const Http::ResponseData& responseData) const
        {
            InputData blob(responseData.Data, INT_MAX);
            // write response headers to request
            blob >> request.OutgoingHeaders;
        }
        
        IProtocol* ProtocolHttp::Process()
        {
            String buffer(Allocator);
            
            char data[REQUEST_BUFFER_SIZE]; // @todo  perhaps use memstack allocator?
            
            // profiling
            TimeValue start, end;
            
            Http::Request request(Allocator, Http::Version::Http_1); // this
            
            start = Timer.GetAsyncTime();
            
            do
            {
                // protocol may change connection parameter under some circumstances (e.g. upgrade request)
                request.ConnectionParams = Http::ConnectionParams::Connection_Close;
                
                RunProtocol(request, data, buffer);
                // clearup after processing request
                request.Clear();
            }
            while (IsConnectionReuse(request));
            
            end = Timer.GetAsyncTime();
            
            LogInfo.log("Skylar") << "Request completed in " << (end - start).GetMilliSeconds() << "ms";
            
            // see docs
            if (IsConnectionLeaveOpen(request))
            {
                LogInfo.log("Skylar") << "Switching to websocket..";
                
                return NEKO_NEW(Allocator, ProtocolWebSocket)(*this);
            }
            
            return this;
        }
      
        const ApplicationSettings* ProtocolHttp::GetApplicationSettingsForRequest(Http::Request& request, const bool secure) const
        {
            // Get domain or address from incoming request
            auto hostIt = request.IncomingHeaders.Find("host");
            
            if (not hostIt.IsValid())
            {
                LogWarning.log("Skylar") << "GetApplicationSettingsForRequest: Request with no host header?!";
                
                return nullptr;
            }
            
            // If domain name is set
            
            // use default port if not set
            const uint16 defaultPort = secure ? DEFAULT_HTTPS_PORT : DEFAULT_HTTP_PORT;
            
            const String& hostHeader = hostIt.value();
            const int32 delimiter = hostHeader.Find(":"); // port
            
            // @todo perhaps if using default ports hostHeader may contain no port
            
            request.Host = (delimiter == INDEX_NONE) ? hostHeader : hostHeader.Mid(0, delimiter);  // name/address
            
            // port value
            const uint16 port = (delimiter != INDEX_NONE)
                ? static_cast<uint16>(::atoi(*hostHeader.Mid(delimiter + 1)))
                : defaultPort;
            
            // get application settings by name
            const ApplicationSettings* applicationSettings = Settings->List.Find(request.Host) ;
            
            // app is found
            if (applicationSettings != nullptr and (applicationSettings->Port == port or applicationSettings->TlsPort == port))
            {
                return applicationSettings;
            }
            
            return nullptr;
        }
        
        static void ParseContentParameters(const String& headerValue, THashMap<String, String>& contentParameters, String& contentTypeName)
        {
            // Check if request data has additional parameters
            int32 delimiter = headerValue.Find(";");
            
            // If there are some..
            if (delimiter != INDEX_NONE)
            {
                contentTypeName = headerValue.Mid(0, delimiter).Trim();
                
                for (int32 paramCur = delimiter + 1, paramEnd = 0; paramEnd != INDEX_NONE; paramCur = paramEnd + 1)
                {
                    paramEnd = headerValue.Find(";",paramCur);
                    delimiter = headerValue.Find("=", paramCur);
                    
                    if (delimiter >= paramEnd)
                    {
                        String paramName = headerValue.Mid(paramCur, (paramEnd != INDEX_NONE) ? paramEnd - paramCur : INT_MAX).Trim();
                        
                        contentParameters.Insert(Neko::Move(paramName), Neko::String()
                                             );
                    }
                    else
                    {
                        String paramName = headerValue.Mid(paramCur, delimiter - paramCur).Trim();
                        
                        ++delimiter;
                        
                        String paramValue = headerValue.Mid(delimiter, (paramEnd != INDEX_NONE) ? paramEnd - delimiter : INT_MAX).Trim();
                        
                        contentParameters.Insert(Neko::Move(paramName), Neko::Move(paramValue));
                    }
                }
            }
            else
            {
                contentTypeName = headerValue;
            }
        }
        
        static inline bool ParseRequestContentType(Http::Request& request, String& stringBuffer, const IContentType* contentTypeData, const String& contentTypeName, const ulong contentLength, const THashMap<String, String>& contentParams, ISocket& socket, IAllocator& allocator)
        {
            auto& requestData = (Http::RequestDataInternal& )request;
            
            void* contentTypeState = contentTypeData->CreateState(requestData, contentParams);
            
            ContentDesc contentDesc
            {
                contentLength,
                0,
                0,
                contentTypeState,
                nullptr,
                contentTypeData,
            };
            
            String contentBuffer(allocator);
            
            if (stringBuffer.Length() <= contentLength)
            {
                contentDesc.BytesReceived = stringBuffer.Length();
                std::swap(contentBuffer, stringBuffer);
            }
            else
            {
                contentBuffer.Assign(stringBuffer, 0, contentLength);
                stringBuffer.Erase(0, contentLength);
                
                contentDesc.BytesReceived = contentBuffer.Length();
            }
            
            // Parse content
            bool result = contentTypeData->Parse(contentBuffer, requestData, &contentDesc);
            
            while (result and contentDesc.FullSize > contentDesc.BytesReceived)
            {
                TArray<char> buffer(allocator);
                
                // minumum
                const uint32 size = 512 * 1024;
                
                const ulong left = contentDesc.FullSize - contentDesc.BytesReceived;
                const bool hasData = left >= size;
                
                buffer.Resize(hasData ? size : left); // @todo possible optimization
                
                const long sizeInBytes = socket.GetPacketBlocking(&buffer[0], buffer.GetSize(), request.Timeout);
                
                if (sizeInBytes <= 0)
                {
                    result = false;
                    break;
                }
                
                contentDesc.BytesReceived += (uint32)sizeInBytes;
                
                contentBuffer.Erase(0, contentBuffer.Length() - contentDesc.LeftBytes);
                contentBuffer.Append(&buffer[0], (uint32)sizeInBytes);
                // reset
                contentDesc.LeftBytes = 0;
                
                result = contentTypeData->Parse(contentBuffer, requestData, &contentDesc);
            }
            
            contentTypeData->DestroyState(contentDesc.State);

            if (result)
            {
                if (contentDesc.LeftBytes)
                {
                    stringBuffer.Assign(contentBuffer, contentBuffer.Length() - contentDesc.LeftBytes, contentBuffer.Length());
                }
            }
            return result;
        }
        
        Http::StatusCode ProtocolHttp::GetRequestData(Http::Request& request, String& stringBuffer,
                                                        const ApplicationSettings& applicationSettings) const
        {
            // Get content type and check if we have any data
            auto it = request.IncomingHeaders.Find("content-type");
            
            if (not it.IsValid())
            {
                // hmm
                if (request.Method != "get")
                {
                    LogWarning.log("Skylar") << "Request with no Content-Type";
                }
                return Http::StatusCode::Empty;
            }
            
            const String& headerValue = it.value();
            
            String contentTypeName(Allocator);
            THashMap<String, String> contentParams(Allocator);
            ParseContentParameters(headerValue, contentParams, contentTypeName);
            
            // Get variant-data by name
            const auto contentTypeIt = Settings->ContentTypes.Find(contentTypeName);
            // Check if we support that one
            if (not contentTypeIt.IsValid())
            {
                LogWarning.log("Skylar") << "Unsupported content-type " << *contentTypeName;
                
                return Http::StatusCode::NotImplemented;
            }
            
            const IContentType* contentTypeData = contentTypeIt.value();
            
            // request length in bytes
            ulong contentLength = 0;
            
            auto contentLengthIt = request.IncomingHeaders.Find("content-length");
            // convert
            if (contentLengthIt.IsValid())
            {
                contentLength = StringToUnsignedLong(*contentLengthIt.value());
            }
            
            // check limits
            if (applicationSettings.RequestMaxSize > 0 /* if max size is set */ and applicationSettings.RequestMaxSize < contentLength)
            {
                LogWarning.log("Skylar") << "Large request " << (uint64)contentLength << "/" << applicationSettings.RequestMaxSize;
                
                return Http::StatusCode::RequestEntityTooLarge;
            }
            
            const bool parsed = ParseRequestContentType(request, stringBuffer, contentTypeData, contentTypeName, contentLength, contentParams, Socket, Allocator);
            
            if (not parsed)
            {
                LogError.log("Skylar") << "Couldn't parse data of content-type " << contentTypeName;
                // eh
                
                // if content-type parser has created some
                request.IncomingFiles.Clear();
                
                return Http::StatusCode::InternalServerError;
            }
            
            // ok
            return Http::StatusCode::Empty;
        }
        
        bool ProtocolHttp::SendHeaders(const Http::StatusCode status, TArray< std::pair<String, String> >& headers,
                                     const int32& timeout, const bool end) const
        {
            String string("HTTP/1.1 ", Allocator);
            string += (int)status;
            
            const auto it = GetStatusList().Find((int)status);
            // header status name
            if (it.IsValid())
            {
                string += " ";
                string += it.value();
            }
            
            string += "\r\n";
            // write headers
            for (auto& header : headers)
            {
                string += header.first;
                string += ": ";
                string += header.second;
                string += "\r\n";
            }
            
            string += "\r\n";
            
            return Socket.SendAllPacketsWait(*string, string.Length(), timeout) > 0;
        }
        
        long ProtocolHttp::SendData(const void* source, ulong size, const int32& timeout, Http::DataCounter* dataCounter/* = nullptr*/) const
        {
            long sendSize = Socket.SendAllPacketsWait(source, size, timeout);
            // check if no error returned
            if (sendSize > 0)
            {
                if (dataCounter != nullptr)
                {
                    dataCounter->SendTotal += sendSize;
                }
            }
            
            return sendSize;
        }
        
        static Http::StatusCode GetRequestHeaders(Http::Request& request, String& stringBuffer)
        {
            PROFILE_FUNCTION()
            
            // If request is empty
            if (stringBuffer.IsEmpty())
            {
                return Http::StatusCode::BadRequest;
            }
            
            // Search for end of header (empty string)
            int32 headersEnd = stringBuffer.Find("\r\n\r\n");
            
            if (headersEnd == INDEX_NONE)
            {
                return Http::StatusCode::BadRequest;
            }
            headersEnd += 2;
            
            int32 strCur = 0;
            // Search for end of first header
            int32 strEnd = stringBuffer.Find("\r\n");
            
            if (strEnd == INDEX_NONE)
            {
                return Http::StatusCode::BadRequest;
            }
            // end of line
            stringBuffer[strEnd] = '\0';
            
            // split request method and request parameters
            int32 delimiter = stringBuffer.Find(" ", strCur);
            
            // get request method (e.g. GET, POST, PUT, DELETE, ...)
            request.Method = stringBuffer.Mid(strCur, delimiter - strCur);
            request.Method.ToLowerInline(); // store as lowercase
            // skip
            delimiter += 1;
            
            // suffix of uri
            int32 uriEnd = stringBuffer.Find(" ", delimiter);
            if (uriEnd == INDEX_NONE)
            {
                uriEnd = strEnd;
                // probably protocol version is lower
            }
            else
            {
                stringBuffer[uriEnd] = '\0';
                int32 version = uriEnd + 6; // skip "HTTP/"
                
                if (version < strEnd)
                {
                    // Get actual protocol version of HTTP
                    //    @todo ?
                    // this is not supported anyways but capatible
                }
            }
            
            // Save full url
            request.Path = stringBuffer.Mid(delimiter, uriEnd - delimiter);
            
            // parse the next header
            strCur = strEnd + 2;
            // search for end of that header
            strEnd = stringBuffer.Find("\r\n", strCur);
            // end of line
            stringBuffer[strEnd] = '\0';
            
            // Get request headers
            for ( ; strCur != headersEnd;
                 strEnd = stringBuffer.Find("\r\n", strCur), stringBuffer[strEnd] = '\0')
            {
                // look for delimiter of header-value
                delimiter = stringBuffer.Find(":", strCur);
                // if delimiter is not found (or somehow is not at the right place)
                if (delimiter < strEnd)
                {
                    String headerName = stringBuffer.Mid(strCur, delimiter - strCur);
                    headerName.ToLowerInline();
                    
                    String headerValue = stringBuffer.Mid(delimiter + 1, strEnd - delimiter - 1).Trim();
                    
                    // save
                    request.IncomingHeaders.Insert(Neko::Move(headerName), Neko::Move(headerValue));
                }
                
                // next line
                strCur = strEnd + 2; // \r\n
            }
            stringBuffer.Erase(0, headersEnd + 2);
            
            // ok
            return Http::StatusCode::Empty;
        }
        
        static bool GetRequest(const ISocket& socket, Http::Request& request, char* buffer, String& stringBuffer)
        {
            // Get request data from client
            long size = 0;
            size = socket.GetPacketBlocking((void* )buffer, REQUEST_BUFFER_SIZE, request.Timeout);
            
            // no content or error
            if (size < 0 and stringBuffer.IsEmpty())
            {
                return false;
            }
            
            if (size > 0)
            {
                stringBuffer.Append(buffer, size);
            }
            return true;
        }
        
        static void SendStatus(const ISocket& socket, const Http::Request& request, const Http::StatusCode statusCode, IAllocator& allocator)
        {
            const auto it = GetStatusList().Find((int)statusCode);
            
            if (it.IsValid())
            {
                String headers("HTTP/1.1 ", allocator);
                headers += (int)statusCode; // the code itself
                headers += " ";
                headers += it.value();  // status name
                headers += "\r\n\r\n";  // skip
                
                socket.SendAllPacketsWait(*headers, headers.Length(), request.Timeout);
            }
        }
        
        static inline void CheckRequestUpgrade(Http::Request& request, bool secure)
        {
            auto outUpgradeIt = request.OutgoingHeaders.Find("upgrade");
            
            if (not outUpgradeIt.IsValid())
            {
                return;
            }
            
            const String& upgrade = outUpgradeIt.value();
            upgrade.ToLowerInline();
            
            LogInfo.log("Skylar") << "Upgrade request to " << *upgrade;
            
            if (upgrade == "h2")
            {
                if (secure)
                {
                    // set protocol
                    request.ProtocolVersion = Http::Version::Http_2;
                    request.ConnectionParams |= Http::ConnectionParams::Connection_Reuse;
                }
            }
            else if (upgrade == "h2c")
            {
                if (not secure)
                {
                    // set protocol
                    request.ProtocolVersion = Http::Version::Http_2;
                    request.ConnectionParams |= Http::ConnectionParams::Connection_Reuse;
                }
            }
            else if (upgrade == "websocket")
            {
                request.ConnectionParams |= Http::ConnectionParams::Connection_LeaveOpen;
            }
        }
        
        static inline void CheckRequestKeepAlive(Http::Request& request)
        {
            LogInfo.log("Skylar") << "keep-alive connection request";
            
            --request.KeepAliveTimeout;
            
            if (request.KeepAliveTimeout > 0)
            {
                request.ConnectionParams |= Http::ConnectionParams::Connection_Reuse;
            }
        }
        
        static inline void ClearRequestItems(Http::Request& request)
        {
            request.IncomingFiles.Clear();
        }
        
        static void GetConnectionParams(Http::Request& request, const bool secure, IAllocator& allocator)
        {
            auto InConnectionIt = request.IncomingHeaders.Find("connection");
            auto outConnectionIt = request.OutgoingHeaders.Find("connection");
            
            // check if incoming/outgoing connection parameters are set
            if (InConnectionIt.IsValid() and outConnectionIt.IsValid())
            {
                const String& connectionIn = InConnectionIt.value();
                const String& connectionOut = outConnectionIt.value();
                
                connectionIn.ToLowerInline();
                connectionOut.ToLowerInline();
                
                // parse by connection parameters by a comma
                TArray<String> incomingParams(allocator);
                connectionIn.ParseIntoArray(incomingParams, ",", false);
                
                // find the connection token
                const int32 index = incomingParams.Find([connectionOut](const String& other) {
                    return other == connectionOut;
                });
                
                if (index != INDEX_NONE)
                {
                    const String& param = incomingParams[index];
                    
                    if (param == "upgrade")
                    {
                        // check upgrade parameters
                        CheckRequestUpgrade(request, secure);
                    }
                    else if (param == "keep-alive")
                    {
                        CheckRequestKeepAlive(request);
                    }
                }
            }
        }
        
        void ProtocolHttp::RunProtocol(Http::Request& request, char* buffer, String& stringBuffer) const
        {
            // these can be null
            assert(this->Settings != nullptr);
            
            if (not GetRequest(Socket, request, buffer, stringBuffer))
            {
                return;
            }
            
            auto errorCode = GetRequestHeaders(request, stringBuffer);
            
            if (errorCode != Http::StatusCode::Empty)
            {
                // send last set status
                SendStatus(Socket, request, errorCode, Allocator);
                return;
            }
            
            const bool secureSession = Socket.GetTlsSession() != nullptr;
            const ApplicationSettings* applicationSettings = GetApplicationSettingsForRequest(request, secureSession);
            
            // If application is not found
            if (applicationSettings == nullptr)
            {
                LogWarning.log("Skylar") << "Couldn't find application for \"" << request.Host << "\"!";
                
                // send last set status
                SendStatus(Socket, request, Http::StatusCode::NotFound, Allocator);
                return;
            }
            
            errorCode = GetRequestData(request, stringBuffer, *applicationSettings);
            
            if (errorCode != Http::StatusCode::Empty)
            {
                // send last set status
                SendStatus(Socket, request, errorCode, Allocator);
                return;
            }
            
            RunApplication(request, *applicationSettings);
            
            ClearRequestItems(request);
            
            // check application state
            
            if (request.ApplicationExitCode == APPLICATION_EXIT_SUCCESS)
            {
                GetConnectionParams(request, secureSession, Allocator);
                
                // will do something only if x-sendfile header is set (or if partial content)
                this->Sendfile(request);
            }
        }
        
        void ProtocolHttp::Close()
        {
            Socket.Close();
        }
    }
}

