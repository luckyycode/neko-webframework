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
//  FormUrlEncoded.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Engine/Utilities/Utilities.h"

#include "Nova/Utils.h"
#include "ContentDesc.h"
#include "FormUrlEncoded.h"

namespace Neko::Skylar
{
    using namespace Neko::Net;
    
    FormUrlencoded::FormUrlencoded(IAllocator& allocator)
        : IContentType(allocator)
    {
        Name = "application/x-www-form-urlencoded";
    }
    
    bool FormUrlencoded::ParseFromBuffer(const Neko::String& buffer, Http::RequestDataInternal& requestData, ContentDesc* contentDesc) const
    {
        if (buffer.IsEmpty())
        {
            return EnsureContentLength(*contentDesc);
        }
        
        for (int32 strPos = 0, strEnd = 0; strEnd != INDEX_NONE; strPos = strEnd + 1)
        {
            // Search next parameter
            strEnd = buffer.Find("&", strPos);
            if (strEnd == INDEX_NONE)
            {
                if (contentDesc->FullSize != contentDesc->BytesReceived)
                {
                    // end of params but we still have something to read
                    
                    // save leftover
                    contentDesc->LeftBytes = buffer.Length() - strPos;
                    return true;
                }
            }
            
            // Search parameter Value
            int32 delimiter = buffer.Find("=", strPos);
            const int32 last = (strEnd == INDEX_NONE) ? INT_MAX : strEnd; // hmmm

            if (delimiter >= last)
            {
                String stringToDecode = buffer.Mid(strPos, (strEnd != INT_MAX) ? strEnd - strPos : INT_MAX);
                // name
                String name(Allocator);
                Util::DecodeUrl(stringToDecode, name);
                // empty Value
                requestData.IncomingData.Insert(Neko::Crc32(*name), String());
            }
            else
            {
                // name
                String name(Allocator);
                Util::DecodeUrl(buffer.Mid(strPos, delimiter - strPos), name);
                ++delimiter;
                
                String stringToDecode = buffer.Mid(delimiter, (strEnd != INDEX_NONE) ? strEnd - delimiter : INT_MAX);
                
                // Value
                String value(Allocator);
                Util::DecodeUrl(stringToDecode, value);
                // Store parameter and Value
                requestData.IncomingData.Insert(Neko::Crc32(*name), Neko::Move(value));
            }
        }
        
        return true;
    }
    
}


