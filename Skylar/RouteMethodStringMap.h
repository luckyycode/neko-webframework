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
//  RouteMethodStringMap.h
//  Neko Framework
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"

namespace Neko::Skylar
{
    // uh oh no other way
    class RouteMethodStringMap : public THashMap<uint32, Http::Method>
    {
    public:
        explicit RouteMethodStringMap(IAllocator& allocator)
            : THashMap<uint32, Http::Method>(allocator)
        {
            Add("match",    Http::Method::Any);
            Add("get",      Http::Method::Get);
            Add("post",     Http::Method::Post);
            Add("put",      Http::Method::Put);
            Add("patch",    Http::Method::Patch);
            Add("delete",   Http::Method::Delete);
            Add("trace",    Http::Method::Trace);
            Add("connect",  Http::Method::Connect);
            Add("patch",    Http::Method::Patch);
        }
        
        void Add(const char* name, Http::Method method)
        {
            Insert(Crc32(name), method);
        }
    };
    
    static RouteMethodStringMap& RouteMethodCache()
    {
        static DefaultAllocator allocator;
        static RouteMethodStringMap cache(allocator);
        return cache;
    }
}
