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
//  ContentDesc.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

namespace Neko::Skylar
{
    struct ContentDesc
    {
        /** State object, can be used to store transient content data. */
        void* State;
        void* Data;
        
        const class IContentType* ContentType;
        
        /** Size of all content. */
        uint32 FullSize;
        /** Parsed content size. */
        int32 BytesReceived;
        /** Leftover */
        uint32 LeftBytes;
        
        char pad[4];
    };
    
    static inline bool EnsureContentLength(ContentDesc& contentDesc)
    {
        return contentDesc.FullSize == 0
            or contentDesc.FullSize != contentDesc.BytesReceived;;
    }
}
