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
//  Utilities.h
//  Neko SDK
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"
#include "Engine/Utilities/NekoString.h"

#include "Engine/Network/Http/Request.h"

namespace Neko::Net::Http { class Response; }
namespace Neko::Skylar
{
    using namespace Neko::Net;
    
    // binary
    static const char* DefaultMimeType = "application/octet-stream";
    
    /**
     * Gets a mime type (e.g. image/jpeg) from file name.
     *
     * @param fileName  File name with extension.
     * @param mimes     Supported mime types.
     */
    String GetMimeByFileName(const String& fileName, const THashMap<uint32, String>& mimes);
    
    /**
     * Parser url with a query to hashmap (e.g. /get?fileId=14&access=read..).
     *
     * @param incomingData  Query parameters will be saved in that map.
     * @param uri           Url to parse.
     */
    void GetIncomingQueryVars(THashMap<uint32, String>& incomingData, const String& uri, IAllocator& allocator);
}

