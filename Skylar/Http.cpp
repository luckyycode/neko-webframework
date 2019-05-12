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
#include "Engine/Network/Http/HttpStatusCodesMap.h"

#include "RouteMethodStringMap.h"
#include "../ContentTypes/ContentDesc.h"
#include "../Sockets/ISocket.h"
#include "../Utils.h"

// Http 1.1 capable protocol

namespace Neko::Skylar
{
    using namespace Neko::Net::Http;

    struct ContentInfo
    {
        const IContentType* ContentTypeData;
        const String& ContentTypeName;
        const ulong ContentLength;
        const THashMap<String, String>& ContentParams;
    };
    
    ProtocolHttp::ProtocolHttp(ISocket& socket, IAllocator& allocator)
        : Protocol(socket, allocator)
    { }
    
    void ProtocolHttp::WriteRequest(char* buffer, const Http::Request& request,
        const PoolApplicationSettings& applicationSettings) const
    {
        OutputData dataStream(reinterpret_cast<void* >(buffer), INT_MAX);

        request.Serialize(dataStream);
        
        // app
        dataStream.WriteString(applicationSettings.RootDirectory);
    }
    
    void ProtocolHttp::ReadResponse(Http::Request& request, const Http::ResponseData& responseData) const
    {
        InputData dataStream(responseData.Data, INT_MAX);
        // write response headers to request
        dataStream >> request.OutgoingHeaders;
    }
    
    Protocol* ProtocolHttp::Process()
    {
        String buffer(Allocator);
        char data[RequestBufferSize]; // @todo  perhaps use memstack Allocator?
        
        // profiling
        TimeValue start, end;
        Http::Request request(Allocator, Http::Version::Http_1); // this
        start = Timer.GetAsyncTime();
        
        do
        {
            // protocol may change connection parameter under some circumstances (e.g. upgrade request)
            request.ConnectionParams = Http::ConnectionParams::Connection_Close;
            
            RunProtocol(request, data, buffer);
            // cleanup after processing request
            request.Clear();
        }
        while (IsConnectionInReuse(request));
        
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
    
    const PoolApplicationSettings* ProtocolHttp::GetApplicationSettingsForRequest(Http::Request& request, const bool secure) const
    {
        // Get domain or address from incoming request
        auto hostIt = request.IncomingHeaders.Find("host");
        
        if (hostIt.IsValid() == false)
        {
            LogWarning.log("Skylar") << "A request with no host header?!";
            
            return nullptr;
        }
        
        // If domain name is set
        // use default port if not set
        const auto defaultPort = uint16(secure ? DefaultHttpsPort : DefaultHttpPort);
        
        const auto& hostHeader = hostIt.value();
        const int32 delimiter = hostHeader.Find(":"); // port
        
        // perhaps if using default ports hostHeader may contain no port
        request.Host = (delimiter == INDEX_NONE)
            ? hostHeader
            : hostHeader.Mid(0, delimiter);  // name/address
        
        // port value
        const uint16 port = (delimiter != INDEX_NONE)
            ? static_cast<uint16>(::atoi(*hostHeader.Mid(delimiter + 1)))
            : defaultPort;
        
        // get application settings by name
        const auto* applicationSettings = SharedSettings->List.Find(request.Host) ;
        
        // lets hope app is found
        return applicationSettings != nullptr
            and (applicationSettings->Port == port or applicationSettings->TlsPort == port)
                ? applicationSettings
                : nullptr;
    }
    
    static void ParseContentParameters(const String& headerValue, THashMap<String, String>& contentParameters,
        String& contentTypeName)
    {
        // check if request data has additional parameters
        int32 delimiter = headerValue.Find(";");
        
        // if there are some..
        if (delimiter != INDEX_NONE)
        {
            contentTypeName = headerValue
                .Mid(0, delimiter)
                .Trim();
            
            for (int32 paramCur = delimiter + 1, paramEnd = 0; paramEnd != INDEX_NONE; paramCur = paramEnd + 1)
            {
                paramEnd = headerValue.Find(";",paramCur);
                delimiter = headerValue.Find("=", paramCur);
                
                if (delimiter >= paramEnd)
                {
                    auto paramName = headerValue
                        .Mid(paramCur, (paramEnd != INDEX_NONE) ? paramEnd - paramCur : INT_MAX)
                        .Trim();
                    
                    contentParameters.Insert(Neko::Move(paramName), Neko::String()
                                             );
                }
                else
                {
                    auto paramName = headerValue.Mid(paramCur, delimiter - paramCur).Trim();
                    ++delimiter;
                    
                    auto paramValue = headerValue
                        .Mid(delimiter, (paramEnd != INDEX_NONE) ? paramEnd - delimiter : INT_MAX)
                        .Trim();
                    
                    contentParameters.Insert(Neko::Move(paramName), Neko::Move(paramValue));
                }
            }
        }
        else
        {
            contentTypeName = headerValue;
        }
    }
    
    static inline bool ParseRequestContentType(Http::Request& request, String& stringBuffer,
       const ContentInfo& contentInfo, ISocket& socket, IAllocator& allocator)
    {
        auto requestData = static_cast<Http::RequestDataInternal& >(request);
        auto contentLength = contentInfo.ContentLength;
        auto* contentTypeData = contentInfo.ContentTypeData;
        auto& contentParams = contentInfo.ContentParams;
        
        auto contentTypeState = contentTypeData->CreateState(requestData, contentParams);
        
        ContentDesc contentDesc
        {
            .State = contentTypeState,
            .Data = nullptr,
            .ContentType = contentTypeData,
            .FullSize = contentLength,
            .BytesReceived = 0,
            .LeftBytes = 0,
        };
        
        String contentBuffer(allocator);
        
        int32 length = stringBuffer.Length();
        if (length <= contentLength)
        {
            contentDesc.BytesReceived = length;
            std::swap(contentBuffer, stringBuffer);
        }
        else
        {
            contentBuffer.Assign(stringBuffer, 0, contentLength);
            stringBuffer.Erase(0, contentLength);
            
            contentDesc.BytesReceived = contentBuffer.Length();
        }
        
        // parse content
        bool result = contentTypeData->ParseFromBuffer(contentBuffer, requestData, &contentDesc);
        while (result and contentDesc.FullSize > contentDesc.BytesReceived)
        {
            TArray<char> buffer(allocator);
            
            // minimum
            const uint32 size = 512 * 1024;
            
            const ulong left = contentDesc.FullSize - contentDesc.BytesReceived;
            const bool hasData = left >= size;
            
            buffer.Resize(hasData ? size : left); // todo possible optimization
            
            const long sizeInBytes = socket.GetPacketBlocking(&buffer[0], buffer.GetSize(), request.Timeout);
            if (sizeInBytes <= 0)
            {
                result = false;
                break;
            }
            
            contentDesc.BytesReceived += static_cast<uint32>(sizeInBytes);
            
            contentBuffer.Erase(0, contentBuffer.Length() - contentDesc.LeftBytes);
            contentBuffer.Append(&buffer[0], static_cast<uint32>(sizeInBytes));
            // reset
            contentDesc.LeftBytes = 0;
            
            result = contentTypeData->ParseFromBuffer(contentBuffer, requestData, &contentDesc);
        }
        
        contentTypeData->DestroyState(contentDesc.State);
        
        if (result)
        {
            if (contentDesc.LeftBytes > 0)
            {
                stringBuffer.Assign(contentBuffer, contentBuffer.Length() - contentDesc.LeftBytes, contentBuffer.Length());
            }
        }

        return result;
    }
    
    Http::StatusCode ProtocolHttp::GetRequestData(Http::Request& request, String& stringBuffer,
        const PoolApplicationSettings& applicationSettings) const
    {
        // get a content type and check if we have any data
        auto it = request.IncomingHeaders.Find("content-type");
        
        if (not it.IsValid())
        {
            // hmm
            if (request.Method != Method::Get)
            {
                LogWarning.log("Skylar") << "Request with no Content-Type";
            }

            return Http::StatusCode::Empty;
        }
        
        const auto& headerValue = it.value();
        
        String contentTypeName(Allocator);
        
        THashMap<String, String> contentParams(Allocator);
        ParseContentParameters(headerValue, contentParams, contentTypeName);
        
        // Get variant-data by name
        const auto contentTypeIt = SharedSettings->ContentTypes.Find(contentTypeName);
        // Check if we support that one
        if (not contentTypeIt.IsValid())
        {
            LogWarning.log("Skylar") << "Unsupported content-type " << *contentTypeName;

            return Http::StatusCode::NotImplemented;
        }
        
        IContentType* contentTypeData = contentTypeIt.value();
        
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
            LogWarning.log("Skylar") << "Large request "
                << (uint64)contentLength << "/" << applicationSettings.RequestMaxSize;

            return Http::StatusCode::RequestEntityTooLarge;
        }
        
        ContentInfo info =
        {
            .ContentTypeData = contentTypeData,
            .ContentTypeName = contentTypeName,
            .ContentLength = contentLength,
            .ContentParams = contentParams,
        };
        
        const bool parsed = ParseRequestContentType(request, stringBuffer, info, Socket, Allocator);
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
    
    bool ProtocolHttp::SendHeaders(const Http::StatusCode status, ListOfHeaderPair& headers,
            const int32& timeout, const bool end) const
    {
        String string("HTTP/1.1 ", Allocator);
        string += (int)status;
        
        const auto it = StatusCodesMap().Find(static_cast<int>(status));
        // header status name
        if (it.IsValid())
        {
            string += " ";
            string += it.value();
        }
        else
        {
            // todo fill all statuses
            assert(false);
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
        const auto sendSize = Socket.SendAllPacketsWait(source, size, timeout);
        // check if no error returned
        if (sendSize > 0 && dataCounter != nullptr)
        {
            dataCounter->SendTotal += sendSize;
        }
        
        return sendSize;
    }
    
    static Http::StatusCode GetRequestHeaders(Http::Request& request, String& stringBuffer)
    {
        PROFILE_FUNCTION();
        
        // if request is empty
        if (stringBuffer.IsEmpty())
        {
            return Http::StatusCode::BadRequest;
        }
        
        // search for end of header (empty string)
        int32 headersEnd = stringBuffer.Find("\r\n\r\n");
        if (headersEnd == INDEX_NONE)
        {
            return Http::StatusCode::BadRequest;
        }
        
        int32 strCur = 0;
        // search for end of first header
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
        auto method = stringBuffer.Mid(strCur, delimiter - strCur);
        method.ToLowerInline(); // store as lowercase
        
        uint32 hash = Crc32(*method, method.Length());
        request.Method = RouteMethodCache()[hash];
        
        // skip
        delimiter += 1;
        
        // suffix of uri
        int32 uriEnd = stringBuffer.Find(" ", delimiter);
        if (uriEnd != INDEX_NONE)
        {
            stringBuffer[uriEnd] = '\0';
            int32 version = uriEnd + 6; // skip "HTTP/"
            if (version < strEnd)
            {
                // todo ? Get actual protocol version of HTTP
                // this is not supported anyways but compatible
            }
        }
        else
        {
            uriEnd = strEnd;

            // probably protocol version is lower
            return Http::StatusCode::HttpVersionNotSupported;
        }
        
        // save the full url
        request.Path = stringBuffer.Mid(delimiter, uriEnd - delimiter);
        
        // parse the next header
        strCur = strEnd + 2;
        // search for end of that header
        strEnd = stringBuffer.Find("\r\n", strCur);
        // end of line
        stringBuffer[strEnd] = '\0';
        
        headersEnd += 2;
        // Get request headers
        for ( ; strCur != headersEnd;
             strEnd = stringBuffer.Find("\r\n", strCur), stringBuffer[strEnd] = '\0')
        {
            // look for delimiter of header-value
            delimiter = stringBuffer.Find(":", strCur);
            // if delimiter is not found (or somehow is not at the right place)
            if (delimiter < strEnd)
            {
                auto headerName = stringBuffer.Mid(strCur, delimiter - strCur);
                headerName.ToLowerInline();
                
                const String headerValue = stringBuffer
                    .Mid(delimiter + 1, strEnd - delimiter - 1)
                    .Trim();
                
                // save
                request.IncomingHeaders.Insert(Neko::Move(headerName), headerValue);
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
        const auto size = socket.GetPacketBlocking(reinterpret_cast<void* >(buffer), RequestBufferSize, request.Timeout);
        
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
    
    static void SendStatus(const ISocket& socket, const Http::Request& request, const Http::StatusCode statusCode,
        IAllocator& allocator)
    {
        const auto it = Http::StatusCodesMap().Find(static_cast<int>(statusCode));
        
        if (not it.IsValid())
        {
            assert(false);
            return;
        }
        
        StaticString<64> headers("HTTP/1.1 ");
        headers << static_cast<int>(statusCode); // the code itself
        headers << " ";
        headers << it.value();  // status name
        headers << "\r\n\r\n";  // skip
        
        socket.SendAllPacketsWait(headers, StringLength(headers), request.Timeout);
    }
    
    static inline void CheckRequestUpgrade(Http::Request& request, bool secure)
    {
        auto outUpgradeIt = request.OutgoingHeaders.Find("upgrade");
        if (not outUpgradeIt.IsValid())
        {
            return;
        }
        
        const auto& upgradeValue = outUpgradeIt.value();
        upgradeValue.ToLowerInline();
        
        LogInfo.log("Skylar") << "Upgrade request to " << *upgradeValue;
        
        if (upgradeValue == "h2")
        {
            if (secure)
            {
                // set protocol
                request.ProtocolVersion = Http::Version::Http_2;
                request.ConnectionParams |= Http::ConnectionParams::Connection_Reuse;
            }
        }
        else if (upgradeValue == "h2c")
        {
            if (not secure)
            {
                // set protocol
                request.ProtocolVersion = Http::Version::Http_2;
                request.ConnectionParams |= Http::ConnectionParams::Connection_Reuse;
            }
        }
        else if (upgradeValue == "websocket")
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
        auto inConnectionIt = request.IncomingHeaders.Find("connection");
        auto outConnectionIt = request.OutgoingHeaders.Find("connection");
        
        // check if incoming/outgoing connection parameters are set
        if (inConnectionIt.IsValid() and outConnectionIt.IsValid())
        {
            const auto& connectionIn = inConnectionIt.value();
            const auto& connectionOut = outConnectionIt.value();
            
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
                const auto& param = incomingParams[index];
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
        assert(this->SharedSettings != nullptr);

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
        const auto* applicationSettings = GetApplicationSettingsForRequest(request, secureSession);
        // if the application is not found
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
            // send the last set status
            SendStatus(Socket, request, errorCode, Allocator);
            return;
        }
        
        RunApplication(request, *applicationSettings);
        ClearRequestItems(request);
        
        // check the application state
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


