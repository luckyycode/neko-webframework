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
//  ProtocolOptions.h
//  Neko SDK
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#pragma once

#include "Protocol.h"

#include "Engine/Containers/HashMap.h"
#include "Engine/Core/Log.h"

#include "../../ContentTypes/ContentTypes.h"

namespace Neko::Skylar
{
    struct ProtocolOptions
    {
        ProtocolOptions(IAllocator& allocator)
            : SupportedMimeTypes(allocator)
            , ContentTypes(allocator)
            , Allocator(allocator)
        {
            
        }
        
        /** Adds content type to supported content types list. */
        void AddContentType(IContentType& contentType)
        {
            LogInfo("Skylar") << "AddContentType \"" << *contentType.GetName() << "\"";
            ContentTypes.Insert(Crc32(*contentType.GetName()), &contentType);
        }
        
        void AddContentType(const char* name, const char* type)
        {
            uint32 hash = Crc32(name);
            SupportedMimeTypes.Insert(hash, type);
        }
        
        void Build()
        {
            // Default mime types
            AddContentType("html", "text/html");
            AddContentType("js", "text/javascript");
            AddContentType("css", "text/css");
            
            AddContentType("gif", "image/gif");
            AddContentType("jpg", "image/jpeg");
            AddContentType("jpeg", "image/jpeg");
            AddContentType("png", "image/png");
            
            AddContentType("webm", "video/webm");
            AddContentType("mp4", "video/mp4");
            AddContentType("3gp", "video/3gp");
            
            // @todo More Data content types (e.g. multipart/form-Data).
            AddContentType(*NEKO_NEW(Allocator, TextPlain) (Allocator));
            AddContentType(*NEKO_NEW(Allocator, FormUrlencoded) (Allocator));
            AddContentType(*NEKO_NEW(Allocator, ApplicationJson) (Allocator));
            
            LogInfo("Skylar") << "Supported mime types:";
            for (auto& type : SupportedMimeTypes)
            {
                LogInfo("Skylar") << "\t" << *type;
            }
        }
        
        void Destroy()
        {
            if (not ContentTypes.IsEmpty())
            {
                for (auto type : ContentTypes)
                {
                    NEKO_DELETE(Allocator, type) ;
                }
                
                ContentTypes.Clear();
            }
        }
        
        /** Supported mime types. */
        THashMap< uint32, String > SupportedMimeTypes;
        /** Supported content variants. */
        THashMap< uint32, IContentType* > ContentTypes;
        
        IAllocator& Allocator;
    };
}
