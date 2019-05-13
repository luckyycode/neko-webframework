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
//  Protocol.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Protocol.h"
#include "../Sockets/ISocket.h"
#include "../ContentTypes/ContentDesc.h"
#include "../Utils.h"

#include "Engine/Core/Profiler.h"
#include "Engine/Network/Http/Response.h"
#include "Engine/Platform/Platform.h"
#include "Engine/FileSystem/PlatformFile.h"

#include "Engine/Core/Log.h"

namespace Neko::Skylar
{
    Protocol::Protocol(ISocket& socket, IAllocator& allocator)
    : Socket(socket)
    , SharedSettings(nullptr)
    , Allocator(allocator)
    , Timer()
    { }

    Protocol::Protocol(const Protocol& protocol)
    : Socket(protocol.Socket)
    , SharedSettings(protocol.SharedSettings)
    , Allocator(protocol.Allocator)
    , Timer()
    { }

    bool Protocol::SendResponse(Http::Response& response, const uint32 timeout) const
    {
        const auto& responseBody = response.GetBodyData();
        
        const uint8* data = responseBody.Value;
        const ulong size = responseBody.Size;
        
        assert(response.Status != Http::StatusCode::Empty);
        
        bool result = SendHeaders(response, nullptr, timeout);
        
        // send Data if have any
        if (result and data != nullptr and size > 0)
        {
            result = SendData(data, size, timeout, nullptr) > 0;
            response.ClearBodyData();
        }

        return result;
    }
    
    bool Protocol::SendHeaders(Http::Response& response, const ListOfHeaderPair* extra, const int32& timeout, const bool end) const
    {
        ListOfHeaderPair headers(Allocator);
        
        const uint32 size = response.Headers.GetSize() + (extra != nullptr ? extra->GetSize() : 0);
        headers.Reserve(size);
        
        for (auto iter = response.Headers.begin(), end = response.Headers.end(); iter != end; ++iter) {
            headers.Emplace(iter.key(), iter.value());
        }
        
        if (extra != nullptr)
        {
            for (auto& iter : *extra) {
                headers.Emplace(iter.first, iter.second);
            }
        }
        return this->SendHeaders(response.Status, headers, timeout, end);
    }
    
    void Protocol::RunApplication(Http::Request& request, const PoolApplicationSettings& applicationSettings) const
    {
        PROFILE_FUNCTION()
        
        char buffer[RequestBufferSize]; // @todo resizable
        //            TArray<char> buffer(Allocator);
        //            buffer.Reserve(RequestBufferSize);
        
        // write headers so application can read these
        WriteRequest(buffer, request, applicationSettings);
        
        // application input
        Http::RequestData requestData
        {
            Socket.GetNativeHandle(),
            Socket.GetTlsSession(),
            buffer
        };
        
        // application output
        Http::ResponseData responseData { nullptr, 0 };
        
        assert(applicationSettings.OnApplicationRequest != nullptr);
       
        // Launch application
        request.ApplicationExitCode = applicationSettings.OnApplicationRequest(&requestData, &responseData);
        // check results
        if (request.ApplicationExitCode == APPLICATION_EXIT_SUCCESS)
        {
            if (responseData.Data != nullptr and responseData.Size > 0)
            {
                ReadResponse(request, responseData);
                
                // Clear application outgoing Data
                applicationSettings.OnApplicationPostRequest(&responseData);
            }
        }
        else
        {
            LogWarning.log("Skylar") << "Application " << *applicationSettings.ServerModulePath << " exited with a error.";
        }
    }
    
    ContentDesc* Protocol::CreateContentDescriptor(const Http::RequestDataInternal& requestData, const THashMap<String, IContentType* >& contentTypes, IAllocator& allocator)
    {
        auto it = requestData.IncomingHeaders.Find("content-type");
        if (not it.IsValid())
        {
            LogInfo.log("Skylar") << "CreateContentDescriptor: No content-type header set.";
            return nullptr;
        }
        
        const auto& header = it.value();
        
        String contentTypeName(allocator);
        THashMap<String, String> contentParams(allocator);
        // Check if request content type has additional Data
        int32 delimiter = header.Find(";");
        
        // we have somethin
        if (delimiter != INDEX_NONE)
        {
            contentTypeName = header.Mid(0, delimiter).Trim();
            
            for (int32 paramCur = delimiter + 1, paramEnd = 0; paramEnd != INDEX_NONE; paramCur = paramEnd + 1)
            {
                paramEnd = header.Find(";", paramCur);
                delimiter = header.Find("=", paramCur);
                
                if (delimiter >= paramEnd)
                {
                    auto paramName = header
                        .Mid(paramCur, (paramEnd != INDEX_NONE) ? paramEnd - paramCur : INT_MAX)
                        .Trim();
                    
                    contentParams.Insert(Neko::Move(paramName), Neko::String());
                }
                else
                {
                    auto paramName = header
                        .Mid(paramCur, delimiter - paramCur)
                        .Trim();
                    
                    ++delimiter;
                    
                    auto paramValue = header
                        .Mid(delimiter, (paramEnd != INDEX_NONE) ? paramEnd - delimiter : INT_MAX)
                        .Trim();
                    
                    contentParams.Insert(Neko::Move(paramName), Neko::Move(paramValue)
                                         );
                }
            }
        }
        else
        {
            contentTypeName = header;
        }
        
        const auto contentTypeIt = contentTypes.Find(contentTypeName);
        if (not contentTypeIt.IsValid())
        {
            return nullptr;
        }
        
        ulong dataLength = 0;
        
        const auto* contentType = contentTypeIt.value();
        if (const auto contentLengthIt = requestData.IncomingHeaders.Find("content-length"); contentLengthIt.IsValid())
        {
            dataLength = StringToUnsignedLong(*contentLengthIt.value());
        }
        
        // transient content Data
        auto* state = contentType->CreateState(requestData, contentParams);
        return NEKO_NEW(allocator, ContentDesc)
        {
            state,
            nullptr,
            contentType,
            dataLength,
            0, 0,
        };
    }
    
    void Protocol::DestroyContentDescriptor(void* source, IAllocator& allocator)
    {
        auto* content = static_cast<ContentDesc* >(source);
        if (content != nullptr)
        {
            content->ContentType->DestroyState(content->State);
            
            if (content->Data)
            {
                NEKO_DELETE(allocator, static_cast<String* >(content->Data)) ;
                content->Data = nullptr;
            }
        }
        NEKO_DELETE(allocator, content) ;
    }
    
    // helpers
    static TArray< Tuple<ulong, ulong> > GetRanges(const String& rangeHeader, const uint32 valueOffset,
        const ulong fileSize, String* resultRangeHeader, ulong* contentLength, IAllocator& allocator)
    {
        // supported range units
        static const THashMap< String, uint32 > rangesUnits({ { "bytes", 1 } }, allocator);
        TArray< Tuple<ulong, ulong> > ranges(allocator);
        
        int32 delimiter = valueOffset;
        const String rangeUnitString(rangeHeader, 0, delimiter);
        
        const auto unitIt = rangesUnits.Find(rangeUnitString);
        if (not unitIt.IsValid())
        {
            LogWarning.log("Skylar") << "Unsupported range unit type \"" << *rangeUnitString << "\".";

            return ranges;
        }
        
        const uint32 rangeUnit = unitIt.value();
        for (int32 strPos; delimiter != INDEX_NONE; )
        {
            strPos = delimiter + 1;
            // bytes=0-1024,1024-2048...
            delimiter = rangeHeader.Find(",", strPos);
            
            // 0-1024
            const int32 rangePos = rangeHeader.Find("-", strPos);
            if (rangePos < delimiter or delimiter == INDEX_NONE)
            {
                int32 c = rangePos - strPos;
                const String rangeStrBegin(rangeHeader, strPos, c);
                
                c = (delimiter == INDEX_NONE)
                    ? rangeHeader.Length()
                    : delimiter - (rangePos + 1);

                const String rangeStrEnd(rangeHeader, rangePos + 1, c);
                if (not rangeStrBegin.IsEmpty())
                {
                    const ulong rangeBeginValue = StringToUnsignedLong(*rangeStrBegin) * rangeUnit;
                    // we have something get
                    if (rangeBeginValue < fileSize)
                    {
                        // end Value is set..
                        if (not rangeStrEnd.IsEmpty())
                        {
                            ulong rangeEndValue = StringToUnsignedLong(*rangeStrEnd) * rangeUnit;
                            // end Value always should have more length than begin Value
                            if (rangeEndValue >= rangeBeginValue)
                            {
                                // clamp
                                if (rangeEndValue > fileSize)
                                {
                                    rangeEndValue = fileSize;
                                }

                                const ulong length = rangeEndValue - rangeBeginValue + 1;
                                *contentLength += length;

                                // build range string
                                resultRangeHeader->Append((uint64)rangeBeginValue);
                                resultRangeHeader->Append("-");
                                resultRangeHeader->Append((uint64)rangeEndValue);
                                resultRangeHeader->Append(",");

                                ranges.Emplace(Tuple< ulong, ulong > { rangeBeginValue, length });
                            }
                        }
                        else // if range end Value is empty, use full file size Value
                        {
                            const ulong length = fileSize - rangeBeginValue;
                            *contentLength += length;

                            // build range string
                            resultRangeHeader->Append((uint64)rangeBeginValue);
                            resultRangeHeader->Append("-");
                            resultRangeHeader->Append((uint64)fileSize - 1);
                            resultRangeHeader->Append(",");

                            ranges.Emplace(Tuple< ulong, ulong > { rangeBeginValue, length });
                        }
                    }
                }
                else if (not rangeStrEnd.IsEmpty())
                {
                    ulong rangeEndValue = StringToUnsignedLong(*rangeStrEnd) * rangeUnit; // convert
                    const ulong length = (rangeEndValue < fileSize) ? fileSize - rangeEndValue : fileSize;
                    const ulong rangeBeginValue = fileSize - length;

                    rangeEndValue = fileSize - rangeBeginValue - 1;

                    *contentLength += length;

                    // build range string
                    resultRangeHeader->Append((uint64)rangeBeginValue);
                    resultRangeHeader->Append("-");
                    resultRangeHeader->Append((uint64)rangeEndValue);
                    resultRangeHeader->Append(",");

                    ranges.Emplace(Tuple< ulong, ulong > { rangeBeginValue, length });
                }
            }
        }
        
        // check if range(s) processed
        if (not ranges.IsEmpty())
        {
            // build range string
            int32 l = resultRangeHeader->Length() - 1;
            (*resultRangeHeader)[l] = '/'; // format
            
            String temp = *resultRangeHeader; // uhh
            
            *resultRangeHeader = "bytes ";
            resultRangeHeader->Append(*temp, temp.Length());
            resultRangeHeader->Append(static_cast<uint64>(fileSize));
        }

        return ranges;
    }
    
    static bool SendPartial(const Protocol& protocol, const Http::Request& request, const String& fileName,
        DateTime fileTime, const ulong fileSize, const String& rangeHeader, ListOfHeaderPair& extraHeaders,
        const THashMap<String, String>& mimeTypes, const bool headersOnly, IAllocator& allocator)
    {
        const int32 valueOffset = rangeHeader.Find("=");
        
        // not set
        if (valueOffset == INDEX_NONE)
        {
            protocol.SendHeaders(Http::StatusCode::BadRequest, extraHeaders, request.Timeout);

            return false;
        }
        
        String contentRangeHeader(allocator);
        
        // content can be big enough, store in bigger type
        // ulong should be enough
        ulong contentLength = 0;
        
        const auto ranges = GetRanges(rangeHeader, valueOffset, fileSize, &contentRangeHeader, &contentLength , allocator);
        
        if (contentLength == 0)
        {
            protocol.SendHeaders(Http::StatusCode::RequestRangeNotSatisfiable, extraHeaders, request.Timeout);

            return false;
        }
        
        // Range(s) transfer
        FileSystem::PlatformFile file;
        if (not file.Open(*fileName, FileSystem::Mode::Read))
        {
            file.Close();
            LogError.log("Skylar") << "SendPartial: Couldn't open file for transfer.";
            protocol.SendHeaders(Http::StatusCode::InternalServerError, extraHeaders, request.Timeout);

            return false;
        }
        
        const auto mimeType = GetMimeByFileName(fileName, mimeTypes);
        
        String contentLengthString(allocator);
        contentLengthString += (uint64)contentLength;
        
        // set headers
        auto kekas = fileTime.ToRfc882();
        extraHeaders.Emplace("content-type", mimeType);
        extraHeaders.Emplace("content-length", contentLengthString);
        extraHeaders.Emplace("content-range", contentRangeHeader);
        extraHeaders.Emplace("accept-ranges", "bytes");
        extraHeaders.Emplace("last-modified", kekas );
        
        // Partial Content
        if (not protocol.SendHeaders(Http::StatusCode::PartialContent, extraHeaders, request.Timeout, headersOnly))
        {
            file.Close();
            LogError.log("Skylar") << "SendPartial: Couldn't send headers";

            return false;
        }
        
        // check if we don't need anything else
        if (headersOnly)
        {
            file.Close();

            return true;
        }
        
        ulong position, length;
        TArray<uint8> buffer(allocator);
        
        Http::DataCounter dataCounter { contentLength, 0 };
        
        for (const auto& range : ranges)
        {
            Neko::Tie(position, length) = range;
            
            const uint32 size = 512 * 1024;
            buffer.Reserve(length < size ? length : size);
            
            file.Seek(ESeekLocator::Begin, position);
            
            ulong sendSizeLeft = length;
            long sendSize;
            long readSize;
            
            do
            {
                if (sendSizeLeft < size)
                {
                    buffer.Reserve(sendSizeLeft);
                }
                
                readSize = file.Read((void* )buffer.GetData(), buffer.GetCapacity());
                sendSize = protocol.SendData(buffer.GetData(), readSize, request.Timeout, &dataCounter);
                sendSizeLeft -= sendSize;
            }
            while (not file.IsEof() and sendSizeLeft and sendSize > 0);
        }
        file.Close();

        return true;
    }
    
    static bool Sendfile(const Protocol& protocol, const Http::Request &request, ListOfHeaderPair& extraHeaders,
         const String& fileName, const ServerSharedSettings& settings, IAllocator& allocator)
    {
        // maybe we only need the info
        const bool headersOnly = request.Method == Method::Head;

        // Current time in Gmt
        const auto now = DateTime::GmtNow().ToRfc882();
        extraHeaders.Emplace("date", now);

        // get file info
        const auto fileInfo = Neko::Platform::GetFileData(*fileName);
        uint64 fileSize;
        DateTime fileModificationTime;
        
        // File is not found or not valid
        if (not fileInfo.IsValid)
        {
            LogInfo.log("Skylar") << "Requested file " << *fileName << " not found.";
            protocol.SendHeaders(Http::StatusCode::NotFound, extraHeaders, request.Timeout);

            return false;
        }
        else
        {
            fileSize = static_cast<uint64>(fileInfo.FileSize);
            fileModificationTime = fileInfo.ModificationTime;
            
            if (fileSize == -1) // @see GetFileData
            {
                return false;
            }
            
            LogInfo.log("Skylar") << "Requested file " << *fileName << " - " << (fileSize / 1024ULL) << " kb.";
        }
        
        // If-Modified header
        
        // if its valid, check file modification time
        if (const auto modifiedIt = request.IncomingHeaders.Find("if-modified-since");
            modifiedIt.IsValid())
        {
            const auto time = DateTime::FromRfc822(*modifiedIt.value());
            if (not time.IsValid())
            {
                protocol.SendHeaders(Http::StatusCode::NotAcceptable, extraHeaders, request.Timeout);

                return false;
            }

            if (fileModificationTime == time)
            {
                protocol.SendHeaders(Http::StatusCode::NotModified, extraHeaders, request.Timeout);

                return false;
            }
        }
        
        const auto& mimeTypes = settings.SupportedMimeTypes;
        
        // range transfer
        if (const auto rangeIt = request.IncomingHeaders.Find("range"); rangeIt.IsValid())
        {
            LogInfo.log("Skylar") << "Range header found, partial transfer...";

            return SendPartial(protocol, request, fileName, fileModificationTime, fileSize,
               rangeIt.value(), extraHeaders, mimeTypes, headersOnly, allocator);
        }
        
        // file transfer
        FileSystem::PlatformFile file;
        if (not file.Open(*fileName, FileSystem::Mode::Read))
        {
            file.Close();
            LogError.log("Skylar") << "Unable open the requested file.";
            protocol.SendHeaders(Http::StatusCode::InternalServerError, extraHeaders, request.Timeout);

            return false;
        }
        
        const auto mimeType = GetMimeByFileName(fileName, mimeTypes);
        // format datetime
        const auto lastModifiedGmt = DateTime::GmtNow().ToRfc882();

        String fileSizeString(allocator);
        fileSizeString += fileSize;
        
        // set headers
        extraHeaders.Emplace("content-type", mimeType);
        extraHeaders.Emplace("content-length", fileSizeString);
        extraHeaders.Emplace("last-modified", lastModifiedGmt);
        extraHeaders.Emplace("accept-ranges", "bytes"); // @todo what if we'll have more unit types
        
        // Send Ok header
        bool end = headersOnly or fileSize == 0;
        if (not protocol.SendHeaders(Http::StatusCode::Ok, extraHeaders, request.Timeout, end))
        {
            file.Close();
            LogError.log("Skylar") << "Unable to send headers";

            return false;
        }
        
        // send a requested file
        if (not headersOnly and fileSize)
        {
            // minimum size
            const uint32 size = 512 * 1024;
            
            TArray<uint8> buffer(allocator);
            buffer.Reserve(fileSize < size ? fileSize : size);
            
            long sendSize;
            long readSize;
            
            Http::DataCounter dataCounter { fileSize, 0 };
            do
            {
                readSize = file.Read((void* )buffer.GetData(), buffer.GetCapacity());
                sendSize = protocol.SendData(buffer.GetData(), readSize, request.Timeout, &dataCounter);
            }
            while (not file.IsEof()
                and (dataCounter.FullSize - dataCounter.SendTotal) and sendSize > 0);
        }
        file.Close();

        return true;
    }
    
    bool Protocol::Sendfile(Http::Request& request) const
    {
        assert(this->SharedSettings != nullptr);
        
        auto sendfileIt = request.OutgoingHeaders.Find("x-sendfile");
        if (not sendfileIt.IsValid())
        {
            return false;
        }
        
        ListOfHeaderPair headers(Allocator);
        if (request.ProtocolVersion == Http::Version::Http_1)
        {
            if (request.IsConnectionInReuse())
            {
                headers.Emplace("connection", "keep-alive");
                
                String value("timeout=5; max=", Allocator);
                value += request.KeepAliveTimeout;
                
                headers.Emplace("keep-alive", value);
            }
            else
            {
                headers.Emplace("connection", "close");
            }
        }

        // doesnt assume that file is sent successfully
        return Skylar::Sendfile(*this, request, headers, sendfileIt.value(), *SharedSettings, Allocator);
    }
    
    void Protocol::SetSettingsSource(const ServerSharedSettings& settings)
    {
        this->SharedSettings = &settings;
    }
}


