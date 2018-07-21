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

#include "../../Engine/Core/Profiler.h"
#include "../../Engine/Network/Http/Response.h"
#include "../../Engine/Platform/Platform.h"
#include "../../Engine/FS/PlatformFile.h"

#include "../../Engine/Core/Log.h"

namespace Neko
{
    namespace Skylar
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
   
        bool IProtocol::SendResponse(Http::Response& response, const uint32 timeout) const
        {
            const auto& responseBody = response.GetBodyData();
            
            const uint8* data = responseBody.Value;
            const ulong size = responseBody.Size;
            
            assert(response.Status != Http::StatusCode::Empty);
            
            bool result = SendHeaders(response, nullptr, timeout);
            
            // send data if have any
            if (result && data != nullptr && size > 0)
            {
                result = SendData(data, size, timeout, nullptr) > 0;
                
                Allocator.Deallocate((void* )data);
            }
            return result;
        }
        
        bool IProtocol::SendHeaders(Http::Response& response, const TArray<std::pair<String, String> >* extra, const int32& timeout, const bool end) const
        {
            TArray<std::pair<String, String> > headers(Allocator);
            
            const uint32 size = response.Headers.GetSize() + (extra != nullptr ? extra->GetSize() : 0);

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
        
        void IProtocol::RunApplication(Http::Request& request, const ApplicationSettings& applicationSettings) const
        {
            PROFILE_FUNCTION()
            
            char buffer[REQUEST_BUFFER_SIZE];
//            TArray<char> buffer(Allocator);
//            buffer.Reserve(REQUEST_BUFFER_SIZE);
            
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
                if (responseData.Data != nullptr && responseData.Size > 0)
                {
                    ReadResponse(request, responseData);
                    
                    // Clear application outgoing data
                    applicationSettings.OnApplicationPostRequest(&responseData);
                }
            }
            else
            {
                LogWarning.log("Skylar") << "Application " << *applicationSettings.ServerModulePath << " exited with error.";
            }
        }
        
        ContentDesc* IProtocol::CreateContentDesc(const Http::RequestDataInternal& requestData, const THashMap<String, IContentType* >& contentTypes, IAllocator& allocator)
        {
            auto it = requestData.IncomingHeaders.Find("content-type");
            
            if (!it.IsValid())
            {
                LogInfo.log("Skylar") << "CreateContentDesc: No content-type header set.";
                return nullptr;
            }
            
            const String& header = it.value();
            
            String contentTypeName(allocator);
            THashMap<String, String> contentParams(allocator);
            
            // Check if request content type has additional data
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
                        String paramName = header.Mid(paramCur, (paramEnd != INDEX_NONE) ? paramEnd - paramCur : INT_MAX).Trim();
                        
                        contentParams.Insert(Neko::Move(paramName), Neko::String::Empty);
                    }
                    else
                    {
                        String paramName = header.Mid(paramCur, delimiter - paramCur).Trim();
                        
                        ++delimiter;
                        
                        String paramValue = header.Mid(delimiter, (paramEnd != INDEX_NONE) ? paramEnd - delimiter : INT_MAX).Trim();
                    
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
            
            if (!contentTypeIt.IsValid())
            {
                return nullptr;
            }
            
            ulong dataLength = 0;
            
            const IContentType* contentType = contentTypeIt.value();
            const auto contentLengthIt = requestData.IncomingHeaders.Find("content-length");
            
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
        
        // helpers
        static TArray< Tuple<ulong, ulong> > GetRanges(const String& rangeHeader, const uint32 valueOffset, const ulong fileSize, String* resultRangeHeader, ulong* contentLength, IAllocator& allocator)
        {
            // supported range units
            static const THashMap< String, uint32 > rangesUnits({ { "bytes", 1 } }, allocator);
            
            TArray< Tuple<ulong, ulong> > ranges(allocator);
            
            int32 delimiter = valueOffset;
            const String rangeUnitString(rangeHeader, 0, delimiter);
            
            const auto unitIt = rangesUnits.Find(rangeUnitString);
            if (!unitIt.IsValid())
            {
                LogWarning.log("Skylar") << "GetRanges: Unsupported range unit type \"" << *rangeUnitString << "\"";
                
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
                
                if (rangePos < delimiter || delimiter == INDEX_NONE)
                {
                    int32 c = rangePos - strPos;
                    const String rangeStrBegin(rangeHeader, strPos, c);
                    
                    c = (delimiter == INDEX_NONE) ? rangeHeader.Length() : delimiter - (rangePos + 1);
                    const String rangeStrEnd(rangeHeader, rangePos + 1, c);
                    
                    
                    if (!rangeStrBegin.IsEmpty())
                    {
                        const ulong rangeBeginValue = StringToUnsignedLong(*rangeStrBegin) * rangeUnit;
                        
                        // we have something get
                        if (rangeBeginValue < fileSize)
                        {
                            // end value is set..
                            if (!rangeStrEnd.IsEmpty())
                            {
                                ulong rangeEndValue = StringToUnsignedLong(*rangeStrEnd) * rangeUnit;
                                // end value always should have more length than begin value
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
                            else // if range end value is empty, use full file size value
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
                    else if (!rangeStrEnd.IsEmpty())
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
            if (!ranges.IsEmpty())
            {
                // build range string
                int32 l = resultRangeHeader->Length() - 1;
                (*resultRangeHeader)[l] = '/'; // format
                
                String temp = *resultRangeHeader; // uhh
                
                *resultRangeHeader = "bytes ";
                resultRangeHeader->Append(*temp, temp.Length());
                resultRangeHeader->Append((uint64)fileSize);
            }
            
            return ranges;
        }
        
        static bool SendPartial(const IProtocol& protocol, const Http::Request& request, const String& fileName, DateTime fileTime, const ulong fileSize, const String& rangeHeader, TArray<std::pair<String, String> >& extraHeaders, const THashMap<String, String>& mimeTypes, const bool headersOnly, IAllocator& allocator)
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
            
            const TArray<Tuple<ulong, ulong> > ranges = GetRanges(rangeHeader, valueOffset, fileSize, &contentRangeHeader, &contentLength , allocator);
            
            if (contentLength == 0)
            {
                protocol.SendHeaders(Http::StatusCode::RequestRangeNotSatisfiable, extraHeaders, request.Timeout);
                
                return false;
            }
            
            // Range(s) transfer
            FS::CPlatformFile file;
            
            if (!file.Open(*fileName, FS::Mode::READ))
            {
                file.Close();
                LogError.log("Skylar") << "SendPartial: Couldn't open file for transfer.";
                
                protocol.SendHeaders(Http::StatusCode::InternalServerError, extraHeaders, request.Timeout);
                
                return false;
            }
            
            const String mimeType = GetMimeByFileName(fileName, mimeTypes);
            
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
            if (!protocol.SendHeaders(Http::StatusCode::PartialContent, extraHeaders, request.Timeout, headersOnly))
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
                buffer.Resize(length < size ? length : size);
                
                file.Seek(ESeekLocator::Begin, position);
                
                ulong sendSizeLeft = length;
                long sendSize;
                long readSize;
                
                do
                {
                    if (sendSizeLeft < size)
                    {
                        buffer.Resize(sendSizeLeft);
                    }
                    
                    readSize = file.Read((void* )buffer.GetData(), buffer.GetSize());
                    sendSize = protocol.SendData(buffer.GetData(), readSize, request.Timeout, &dataCounter);
                    sendSizeLeft -= sendSize;
                }
                while (!file.IsEof() && sendSizeLeft && sendSize > 0);
            }
            
            file.Close();
            
            return true;
        }
        
        static bool Sendfile(const IProtocol& protocol, const Http::Request &request, TArray<std::pair<String, String> >& extraHeaders, const String& fileName, const THashMap<String, String>& mimeTypes, const bool headersOnly, IAllocator& allocator)
        {
            // Current time in Gmt
            const auto now = DateTime::GmtNow().ToRfc882();
            extraHeaders.Emplace("date", now);
            
            // get file info
            const auto fileInfo = Neko::Platform::GetFileData(*fileName);
            
            uint64 fileSize;
            DateTime fileModificationTime;
            
            // File is not found or not valid
            if (!fileInfo.bIsValid)
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
            const auto modifiedIt = request.IncomingHeaders.Find("if-modified-since");
            
            // if its valid, check file moditification time
            if (modifiedIt.IsValid())
            {
                const auto requestTime = DateTime::FromRfc822(*modifiedIt.value());
                
                if (fileModificationTime == requestTime)
                {
                    protocol.SendHeaders(Http::StatusCode::NotModified, extraHeaders, request.Timeout);
                    
                    return false;
                }
            }
            
            // Range transfer
            const auto rangeIt = request.IncomingHeaders.Find("range");
            
            if (rangeIt.IsValid())
            {
                LogInfo.log("Skylar") << "Range header found, partial transfer...";
                
                return SendPartial(protocol, request, fileName, fileModificationTime, fileSize, rangeIt.value(), extraHeaders, mimeTypes, headersOnly, allocator);
            }
            
            // File transfer
            FS::CPlatformFile file;
            
            if (!file.Open(*fileName, FS::Mode::READ))
            {
                file.Close();
                LogError.log("Skylar") << "Couldn't open requested file!";
                
                protocol.SendHeaders(Http::StatusCode::InternalServerError, extraHeaders, request.Timeout);
                
                return false;
            }
            
            const String mimeType = GetMimeByFileName(fileName, mimeTypes);
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
            bool end = headersOnly || fileSize == 0;
            if (!protocol.SendHeaders(Http::StatusCode::Ok, extraHeaders, request.Timeout, end))
            {
                file.Close();
                LogError.log("Skylar") << "Failed to send headers";
                
                return false;
            }
            
            // send a requested file
            if (!headersOnly && fileSize)
            {
                // minimum size
                const uint32 size = 512 * 1024;
                
                TArray<uint8> buffer(allocator);
                buffer.Resize(fileSize < size ? fileSize : size);
                
                long sendSize;
                long readSize;
                
                Http::DataCounter dataCounter { fileSize, 0 };
                
                do
                {
                    readSize = file.Read((void* )buffer.GetData(), buffer.GetSize());
                    sendSize = protocol.SendData(buffer.GetData(), readSize, request.Timeout, &dataCounter);
                }
                while (!file.IsEof() && (dataCounter.FullSize - dataCounter.SendTotal) && sendSize > 0);
            }
            
            file.Close();
            
            return true;
        }
        
        bool IProtocol::Sendfile(Http::Request& request) const
        {
            assert(this->Settings != nullptr);
            
            auto sendfileIt = request.OutgoingHeaders.Find("x-sendfile");
            
            if (sendfileIt.IsValid())
            {
                TArray< std::pair<String, String> > headers(Allocator);
                
                if (request.ProtocolVersion == Http::Version::Http_1)
                {
                    if (IsConnectionReuse(request))
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
                
                // maybe we only need the info
                const bool headersOnly = (request.Method == "head");
                
                const auto& mimeTypes = Settings->SupportedMimeTypes;
                // doesnt assume that file is sent successfully
                bool success = Skylar::Sendfile(*this, request, headers, sendfileIt.value(), mimeTypes, headersOnly, Allocator);
                NEKO_UNUSED(success)
                
                return true;
            }
            
            return false;
        }
    }
}

