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
//  FormUrlEncoded.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "IContentType.h"

namespace Neko::Skylar
{
    /** application/x-www-form-urlencoded */
    struct FormUrlencoded : public IContentType
    {
        FormUrlencoded(IAllocator& allocator);
        bool ParseFromBuffer(const String& buffer, Net::Http::RequestDataInternal& requestData, ContentDesc* contentDesc) const override;
    };
}

