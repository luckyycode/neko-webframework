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
//  IContentType.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/NekoString.h"
#include "Engine/Utilities/Templates.h"
#include "Engine/Network/NetSocketBase.h"
#include "Engine/Network/Http/Request.h"

namespace Neko::Skylar
{
    using namespace Neko::Net;
    /** Content type interface. Used for types, e.g. text/plain, application/json, multipart/form-data and so on. */
    class IContentType
    {
    public:
        IContentType(IAllocator& allocator);
        
        /**
         * Virtual destructor
         */
        virtual ~IContentType() = default;
        
        /** Creates transient content data state. */
        virtual void* CreateState(const Net::Http::RequestDataInternal& requestData,
            const Neko::THashMap< Neko::String, Neko::String >& contentParams) const;
        /** Destroys transient content data state. */
        virtual void DestroyState(void* state) const;
        
        /**
         * Parses content type from a data buffer.
         *
         * @param buffer        String data buffer.
         * @param requestData   Parsed data will be saved in request.
         * @param contentDesc   Used to keep data on track.
         */
        virtual bool ParseFromBuffer(const Neko::String& buffer, Net::Http::RequestDataInternal& requestData,
            class ContentDesc* contentDesc) const = 0;
      
    public:
        /** Readable Content-Type */
        const Neko::String& GetName() const;
        
    protected:
        /** Content-Type. */
        Neko::String Name;
        /** Allocator to keep track on created objects. */
        IAllocator& Allocator;
    };
}

