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
//  Files.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Files.h"
#include "Utils.h"
#include "../Engine/Mt/Thread.h"
#include "../Engine/Network/Http/Request.h"
#include "../Engine/Platform/Platform.h"
#include "../Engine/FS/PlatformFile.h"
#include "../Engine/Core/Log.h"

namespace Neko
{
    namespace Http
    {
        bool SendfileExtension::Send(const IServerProtocol &protocol, const Net::Http::Request &request, TArray<std::pair<String, String> >& extraHeaders, const String& fileName, const THashMap<String, String>& mimeTypes, const bool headersOnly, IAllocator& allocator)
        {
            // Current time in Gmt
            const auto now = CDateTime::GmtNow().ToRfc882();
            extraHeaders.Emplace("date", now);
            
            // get file info
            auto fileInfo = Neko::Platform::GetFileData(*fileName);
            
            uint64 fileSize;
            CDateTime fileModificationTime;
            
            // File is not found or not valid
            if (!fileInfo.bIsValid)
            {
                GLogInfo.log("Http") << "Send: Requested file " << *fileName << " not found.";
                protocol.SendHeaders(Net::Http::StatusCode::NotFound, extraHeaders, request.Timeout);
                
                return false;
            }
            else
            {
                fileSize = fileInfo.FileSize;
                fileModificationTime = fileInfo.ModificationTime;
                GLogInfo.log("Http") << "Send: Requested file " << *fileName << " - " << (fileSize / 1024UL) << " kb.";
            }
            
            // If-Modified header
            const auto modifiedIt = request.IncomingHeaders.Find("if-modified-since");
            
            // if its valid, check file moditification time
            if (modifiedIt.IsValid())
            {
                const auto requestTime = CDateTime::FromRfc822(*modifiedIt.value());
                
                if (fileModificationTime == requestTime)
                {
                    protocol.SendHeaders(Net::Http::StatusCode::NotModified, extraHeaders, request.Timeout);
                    
                    return false;
                }
            }
            
            // Range transfer
            const auto rangeIt = request.IncomingHeaders.Find("range");
            
            if (rangeIt.IsValid())
            {
                GLogInfo.log("Http") << "Send: Range header found, partial transfer...";
                
                return SendPartial(protocol, request, fileName, fileModificationTime, fileSize, rangeIt.value(), extraHeaders, mimeTypes, headersOnly, allocator);
            }
            
            // File transfer
            FS::CPlatformFile file;
            
            if (!file.Open(*fileName, FS::Mode::READ))
            {
                file.Close();
                GLogError.log("Http") << "Send: Couldn't open requested file!";
                
                protocol.SendHeaders(Net::Http::StatusCode::InternalServerError, extraHeaders, request.Timeout);
                
                return false;
            }
            
            const String mimeType = GetMimeByFileName(fileName, mimeTypes);
            // format datetime
            const auto lastModifiedGmt = CDateTime::GmtNow().ToRfc882();
            
            String fileSizeString(allocator);
            fileSizeString += fileSize;
            
            // set headers
            extraHeaders.Emplace("content-type", mimeType);
            extraHeaders.Emplace("content-length", fileSizeString);
            extraHeaders.Emplace("last-modified", lastModifiedGmt);
            extraHeaders.Emplace("accept-ranges", "bytes"); // @todo what if we'll have more unit types
            
            // Send Ok header
            bool end = headersOnly || fileSize == 0;
            if (!protocol.SendHeaders(Net::Http::StatusCode::Ok, extraHeaders, request.Timeout, end))
            {
                file.Close();
                GLogError.log("Http") << "Send: Failed to send headers";
                
                return false;
            }
            
            // send a requested file
            if (!headersOnly && fileSize)
            {
                // minimum size
                const uint32 size = 512 * 1024;
                
                TArray<char> buffer(allocator);
                buffer.Resize(fileSize < size ? fileSize : size);
                
                long sendSize;
                long readSize;
                
                Net::Http::DataCounter dataCounter { fileSize, 0 };
                
                do
                {
                    readSize = file.Read((void* )buffer.GetData(), buffer.GetSize());
                    sendSize = protocol.SendData(buffer.GetData(), readSize, request.Timeout, &dataCounter);
                }
                while (!file.IsEof() && (dataCounter.FullSize - dataCounter.SendTotal) && sendSize > 0);
                
                GLogInfo.log("Http") << "Send: Sent " << (uint32)sendSize << "/" << (uint32)fileSize << " kb.";
            }
            
            file.Close();
            
            return true;
        }
        
        // helpers
        
        static TArray< Tuple<size_t, size_t> > GetRanges(const String& rangeHeader, const uint32 valuePos, const uint32 fileSize, String* resultRangeHeader, size_t* contentLength, IAllocator& allocator)
        {
            // supported range units
            static const THashMap< String, size_t > rangesUnits({ { "bytes", 1 } }, allocator);
            
            TArray< Tuple<size_t, size_t> > ranges(allocator);
            
            int32 delimiter = valuePos;
            const String rangeUnitString(rangeHeader, 0, delimiter);
            
            const auto unitIt = rangesUnits.Find(rangeUnitString);
            if (!unitIt.IsValid())
            {
                GLogWarning.log("Http") << "GetRanges: Unsupported range unit type \"" << *rangeUnitString << "\"";
                return ranges;
            }
            
            const uint32 rangeUnit = unitIt.value();
            
            for (int32 strPos; delimiter != INDEX_NONE; )
            {
                strPos = delimiter + 1;
                // bytes=0-1024,1024-2048...
                delimiter = rangeHeader.Find(",", ESearchCase::IgnoreCase, ESearchDir::FromStart, strPos);
                
                // 0-1024
                const int32 rangePos = rangeHeader.Find("-", ESearchCase::IgnoreCase, ESearchDir::FromStart, strPos);
                
                if (rangePos < delimiter || delimiter == INDEX_NONE)
                {
                    int32 c = rangePos - strPos;
                    const String rangeStrBegin(rangeHeader, strPos, c);
                    
                    c = (delimiter == INDEX_NONE) ? rangeHeader.Length() : delimiter - (rangePos + 1);
                    const String rangeStrEnd(rangeHeader, rangePos + 1, c);
                    
                    
                    if (!rangeStrBegin.IsEmpty())
                    {
                        const size_t rangeBeginValue = ::strtoul(*rangeStrBegin, nullptr, 10) * rangeUnit;
                        
                        // we have something get
                        if (rangeBeginValue < fileSize)
                        {
                            // end value is set..
                            if (!rangeStrEnd.IsEmpty())
                            {
                                size_t rangeEndValue = ::strtoul(*rangeStrEnd, nullptr, 10) * rangeUnit;
                                // end value always should have more length than begin value
                                if (rangeEndValue >= rangeBeginValue)
                                {
                                    // clamp
                                    if (rangeEndValue > fileSize)
                                    {
                                        rangeEndValue = fileSize;
                                    }
                                    
                                    const size_t length = rangeEndValue - rangeBeginValue + 1;
                                    
                                    *contentLength += length;
                                    
                                    // build range string
                                    resultRangeHeader->Append((uint64)rangeBeginValue);
                                    resultRangeHeader->Append("-");
                                    resultRangeHeader->Append((uint64)rangeEndValue);
                                    resultRangeHeader->Append(",");
                                    
                                    ranges.Emplace(Tuple< size_t, size_t > { rangeBeginValue, length });
                                }
                            }
                            else // if range end value is empty, use full file size value
                            {
                                const size_t length = fileSize - rangeBeginValue;
                                
                                *contentLength += length;
                                
                                // build range string
                                resultRangeHeader->Append((uint64)rangeBeginValue);
                                resultRangeHeader->Append("-");
                                resultRangeHeader->Append((uint64)fileSize - 1);
                                resultRangeHeader->Append(",");
                                
                                ranges.Emplace(Tuple< size_t, size_t > { rangeBeginValue, length });
                            }
                        }
                    }
                    else if (!rangeStrEnd.IsEmpty())
                    {
                        size_t rangeEndValue = ::strtoul(*rangeStrEnd, nullptr, 10) * rangeUnit; // convert
                        
                        const size_t length = (rangeEndValue < fileSize) ? fileSize - rangeEndValue : fileSize;
                        const size_t rangeBeginValue = fileSize - length;
                        
                        rangeEndValue = fileSize - rangeBeginValue - 1;
                        
                        *contentLength += length;
                        
                        // build range string
                        resultRangeHeader->Append((uint64)rangeBeginValue);
                        resultRangeHeader->Append("-");
                        resultRangeHeader->Append((uint64)rangeEndValue);
                        resultRangeHeader->Append(",");
                        
                        ranges.Emplace(Tuple<size_t, size_t> { rangeBeginValue, length });
                    }
                }
            }
            
            // check if range(s) processed
            if (!ranges.IsEmpty())
            {
                // build range string
                uint32 l = resultRangeHeader->Length() - 1;
                (*resultRangeHeader)[l] = '/'; // format
                
                String temp = *resultRangeHeader; // uhh
                
                *resultRangeHeader = "bytes ";
                resultRangeHeader->Append(*temp, temp.Length());
                resultRangeHeader->Append((uint64)fileSize);
            }
            
            return ranges;
        }
        
        bool SendfileExtension::SendPartial(const IServerProtocol& protocol, const Net::Http::Request& request, const String& fileName, CDateTime fileTime, const size_t fileSize, const String& rangeHeader, TArray<std::pair<String, String> >& extraHeaders, const THashMap<String, String>& mimeTypes, const bool headersOnly, IAllocator& allocator)
        {
            const int32 valuePos = rangeHeader.Find("=", ESearchCase::IgnoreCase, ESearchDir::FromStart);
            
            // not set
            if (valuePos == INDEX_NONE)
            {
                protocol.SendHeaders(Net::Http::StatusCode::BadRequest, extraHeaders, request.Timeout);
                
                return false;
            }
            
            String contentRangeHeader(allocator);
            
            // content can be big enough, store in bigger type
            size_t contentLength = 0;
            
            const TArray<Tuple<size_t, size_t> > ranges = GetRanges(rangeHeader, valuePos, fileSize, &contentRangeHeader, &contentLength , allocator);
            
            if (contentLength == 0)
            {
                protocol.SendHeaders(Net::Http::StatusCode::RequestRangeNotSatisfiable, extraHeaders, request.Timeout);
                
                return false;
            }
            
            // Range(s) transfer
            FS::CPlatformFile file;
            
            if (!file.Open(*fileName, FS::Mode::READ))
            {
                file.Close();
                GLogError.log("Http") << "SendPartial: Couldn't open file for transfer.";
                
                protocol.SendHeaders(Net::Http::StatusCode::InternalServerError, extraHeaders, request.Timeout);
                
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
            if (!protocol.SendHeaders(Net::Http::StatusCode::PartialContent, extraHeaders, request.Timeout, headersOnly))
            {
                file.Close();
                GLogError.log("Http") << "SendPartial: Couldn't send headers";
                
                return false;
            }
            
            // check if we don't need anything else
            if (headersOnly)
            {
                file.Close();
                return true;
            }
            
            size_t position, length;
            TArray<char> buffer(allocator);
            
            Net::Http::DataCounter dataCounter { contentLength, 0 };
            
            for (const auto& range : ranges)
            {
                Neko::Tie(position, length) = range;
                
                const uint32 size = 512 * 1024;
                buffer.Resize(length < size ? length : size);
                
                file.Seek(ESeekLocator::Begin, position);
                
                long sendSizeLeft = length;
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
        bool SendfileExtension::Send(const IServerProtocol& protocol, Net::Http::Request& request, const THashMap<String, String>& mimeTypes, IAllocator& allocator)
        {
            auto sendfileIt = request.OutgoingHeaders.Find("x-sendfile");
            
            if (sendfileIt.IsValid())
            {
                TArray< std::pair<String, String> > headers(allocator);
                
                if (request.ProtocolVersion == Net::Http::Version::Http_1)
                {
                    if (IsConnectionReuse(request))
                    {
                        headers.Emplace("connection", "keep-alive");
            
                        String value("timeout=5; max=", allocator);
                        value += (uint32)request.KeepAliveTimeout;
                        
                        headers.Emplace("keep-alive", value);
                    }
                    else
                    {
                        headers.Emplace("connection", "close");
                    }
                }
                
                // maybe we only need the info
                const bool headersOnly = (request.Method == "head");
                
                // doesnt assume that file is sent successfully
                bool success = Send(protocol, request, headers, sendfileIt.value(), mimeTypes, headersOnly, allocator);
                NEKO_UNUSED(success)
                
                return true;
            }
            
            return false;
        }
    }
}

