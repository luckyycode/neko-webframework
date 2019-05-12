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
//  ApplicationJson.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "ApplicationJson.h"
#include "ContentDesc.h"

namespace Neko::Skylar
{
    ApplicationJson::ApplicationJson(IAllocator& allocator)
    : IContentType(allocator)
    {
        Name = "application/json";
    }
    
    bool ApplicationJson::ParseFromBuffer(const Neko::String& buffer, Http::RequestDataInternal& requestData, ContentDesc* contentDesc) const
    {
        if (buffer.IsEmpty())
        {
            return EnsureContentLength(*contentDesc);
        }
        
        requestData.IncomingData.Insert("jsonData", buffer);
        
        contentDesc->LeftBytes = 0;
        contentDesc->BytesReceived = contentDesc->FullSize;
        
        return true;
    }
}

