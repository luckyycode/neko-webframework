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
//  IContentType.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "IContentType.h"

namespace Neko::Skylar
{
    IContentType::IContentType(IAllocator& allocator)
        : Allocator(allocator)
    {
    }
    
    const String& IContentType::GetName() const
    {
        return this->Name;
    }
    
    void* IContentType::CreateState(const Http::RequestDataInternal& requestData ,
        const THashMap<String, String>& contentParams) const
    {
        return nullptr;
    }
    
    void IContentType::DestroyState(void* state) const
    {
    }
}

