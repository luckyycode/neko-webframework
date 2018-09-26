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
//  Utils.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"
#include "Engine/Utilities/NekoString.h"

#include "Engine/Network/Http/Request.h"

namespace Neko
{
    using namespace Neko::Net;
    
    namespace Net {
        namespace Http {
            class Response;
        }
    }
    
	namespace Skylar
    {
        // binary
        static const char* DefaultMimeType = "application/octet-stream";
        
        /**
         * Gets mime type (e.g. image/jpeg) from file name.
         *
         * @param fileName  File name with extension.
         * @param mimes     Supported mime types.
         */
        String GetMimeByFileName(const String& fileName, const THashMap<String, String>& mimes);
        
        /**
         * Parser url with query to hashmap (e.g. /get?fileId=14&access=read..).
         *
         * @param incomingData  Query parameters will be saved in that map.
         * @param uri           Url to parse.
         */
        void GetIncomingQueryVars(THashMap<String, String>& incomingData, const String& uri, IAllocator& allocator);
        
        /**
         * Removes query params from url.
         *
         * @param path  Input path.
         * @param clean Output.
         */
        void ClearRequestUri(const String& path, String& clean);
      
        /**
         * Shows directory representation in html.
         */
        void ShowDirectoryList(const String& documentRoot, const Net::Http::Request& request, Net::Http::Response& response, bool secure, IAllocator& allocator);
        
        
        NEKO_FORCE_INLINE bool IsConnectionLeaveOpen(const Http::Request& request)
        {
            return (request.ConnectionParams & Net::Http::ConnectionParams::Connection_LeaveOpen)
            == Net::Http::ConnectionParams::Connection_LeaveOpen;
        }
        
        NEKO_FORCE_INLINE bool IsConnectionInReuse(const Http::Request& request)
        {
            return (request.ConnectionParams & Net::Http::ConnectionParams::Connection_Reuse)
            == Net::Http::ConnectionParams::Connection_Reuse;
        }
    }
}
