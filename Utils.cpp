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
//  Utils.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Utils.h"

#include "Engine/Network/Http/Request.h"
#include "Engine/Network/Http/Response.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Utilities/Templates.h"
#include "Engine/Utilities/Utilities.h"
#include "Engine/Core/Log.h"

namespace Neko::Skylar
{
    using namespace Neko::Net;

    void ClearRequestUri(const String& path, String& clean)
    {
        const int32 index = FindFirstOf(*path, "?#");
        
        Util::DecodeUrl(index == INDEX_NONE ? path : path.Mid(0, index), clean);
    }
    
    void ShowDirectoryList(const String& documentRoot, const Http::Request& request, Http::Response& response, bool secure, IAllocator& allocator)
    {
        auto fullHost = request.IncomingHeaders.Find("host"); // get host with port (request.Host may have no port)
        const char* hostString = *fullHost.value();
        
        String body(allocator);
        body += R"(
            <head>
                <style>
                body {
                    background-color: #7E7D7D;
                }
                </style>
            </head>
        
            <body>
                <p style="color:white"> Exploring: <i><font size=4>)";
        
                body += documentRoot;
        
                body += R"(</font></i>
                <table style="width:75%">
                    <tr>
                        <th style="text-align:left">Name</th>
                        <th style="text-align:left">Last modified</th>
                        <th style="text-align:left">Size</th>
                    </tr><hr>
            )";
        
        Platform::FileInfo info;
        Platform::FileStatData fileStat;
        bool isDirectory;
        
        auto* it = Platform::CreateFileIterator(*documentRoot, allocator);
        
        while (Neko::Platform::GetNextFile(it, &info))
        {
            StaticString<MAX_PATH_LENGTH> fullPath(*documentRoot, "/", info.Filename);
            fileStat = Platform::GetFileData(*fullPath);
            
            if (info.Filename[0] == '.')
            {
                continue;
            }
            
            isDirectory = fileStat.bIsDirectory;
            
            // name
            String fileUrl = secure ? "https://" : "http://";
            fileUrl += hostString;
            fileUrl += request.Path;
            fileUrl += "/";
            fileUrl += info.Filename;
            
            body += "<tr><td><a href=\"";
            body += fileUrl;
            body += "\">";
            body += info.Filename;
            body += "</a></td>";
            
            // last modification
            body += "<td>";
            body += fileStat.ModificationTime.ToString("%d-%m-%y %H:%M");
            body += "</td>";
            
            // size
            if (not isDirectory)
            {
                body += "<td>";
                char unit[4];
                Neko::BestUnit(unit, fileStat.FileSize); // size
                body += unit;
                body += "</td></tr>";
            }
            else
            {
                body += "<td></td></tr>";
            }
        }
        Platform::DestroyFileIterator(it);
        
        body += R"(</table></p><hr>
            <footer>
                <p>
                    <a href="https://github.com/luckyycode/neko-webframework">Powered by <i>Neko Framework</i></a>
                </p>
            </footer>
        </body>)";
        
        response.SetStatusCode(Http::StatusCode::Ok);
        response.SetBodyData((uint8* )*body, body.Length());
    }
    
    String GetMimeByFileName(const String& fileName, const THashMap<String, String>& mimes)
    {
        // find extension start
        
        if (const int32 extensionPos = fileName.Find("."); extensionPos != INDEX_NONE)
            // @todo possible optimization? FromEnd
        {
            String extension = fileName.Mid(extensionPos + 1 /* skip dot */).ToLower() ;
            
            if (const auto mimeIt = mimes.Find(extension); mimeIt.IsValid())
            {
                return mimeIt.value();
            }
        }
        
        return String(DefaultMimeType);
    }
    
    void GetIncomingQueryVars(THashMap<String, String>& incomingData, const String& uri, IAllocator& allocator)
    {
        // according to doc only POST and PUT requests should check content-type/content-length info
        // but well...
        
        // Parse URI query params
        
        const int32 start = uri.Find("?");
        
        if (start != INDEX_NONE)
        {
            const int32 finish = uri.Find("#");
            
            // # is missing or found and must be at the end..
            if (finish == INDEX_NONE or finish > start)
            {
                for (int32 paramCur = start + 1, paramEnd = 0; paramEnd != INDEX_NONE; paramCur = paramEnd + 1)
                {
                    paramEnd = uri.Find("&", paramCur);
                    
                    if (finish != INDEX_NONE && paramEnd > finish)
                    {
                        paramEnd = INDEX_NONE;
                    }
                    
                    int32 delimiter = uri.Find("=", paramCur);
                    
                    if (delimiter >= paramEnd)
                    {
                        String name(allocator);
                        Util::DecodeUrl(uri.Mid(paramCur, INDEX_NONE != paramEnd ? paramEnd - paramCur : INT_MAX), name);
                        
                        incomingData.Insert(Neko::Move(name), Neko::String());
                    }
                    else
                    {
                        String name(allocator);
                        Util::DecodeUrl(uri.Mid(paramCur, delimiter - paramCur), name);
                        
                        ++delimiter;
                        
                        String value(allocator);
                        Util::DecodeUrl(uri.Mid(delimiter, paramEnd != INDEX_NONE ? paramEnd - delimiter : INT_MAX), value);
                        
                        incomingData.Insert(Neko::Move(name), Neko::Move(value));
                    }
                }
            }
        }
    }
}

