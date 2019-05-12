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
//  ApplicationJson.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "IContentType.h"

namespace Neko::Skylar
{
    /** Application/json extension for content type. */
    class ApplicationJson: public IContentType
    {
    public:
        ApplicationJson(IAllocator& allocator);
        
    public:
        virtual bool ParseFromBuffer(const String& buffer, Http::RequestDataInternal& requestData, ContentDesc* contentDesc) const override;
    };
}
