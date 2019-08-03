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
//  Utilities.cpp
//  Neko SDK
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#include "Utilities.h"

#include "Engine/Network/Http/Request.h"
#include "Engine/Network/Http/Response.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Utilities/Templates.h"
#include "Engine/Utilities/Utilities.h"
#include "Engine/Core/Log.h"

namespace Neko::Skylar
{
    using namespace Neko::Net;

    String GetMimeByFileName(const String& fileName, const THashMap<uint32, String>& mimes)
    {
        // find extension start
        
        if (const int32 extensionPos = fileName.Find("."); extensionPos != INDEX_NONE)
            // @todo possible optimization? FromEnd
        {
            String extension = fileName.Mid(extensionPos + 1 /* skip dot */).ToLower() ;
            
            if (const auto mimeIt = mimes.Find(Crc32(*extension)); mimeIt.IsValid())
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
        
        // FromString URI query params
        
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

