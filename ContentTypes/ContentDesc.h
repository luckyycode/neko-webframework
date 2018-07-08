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
//  _   _      _           _____                                            _
// | \ | | ___| | _____   |  ___| __ __ _ _ __ ___   _____      _____  _ __| | __
// |  \| |/ _ \ |/ / _ \  | |_ | '__/ _` | '_ ` _ \ / _ \ \ /\ / / _ \| '__| |/ /
// | |\  |  __/   < (_) | |  _|| | | (_| | | | | | |  __/\ V  V / (_) | |  |   <
// |_| \_|\___|_|\_\___/  |_|  |_|  \__,_|_| |_| |_|\___| \_/\_/ \___/|_|  |_|\_\
//
//  ContentDesc.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

namespace Neko
{
    namespace Http
    {
        struct ContentDesc
        {
            // uint32 should fit..
            
            //! Size of all content.
            uint32 FullSize;
            //! Parsed content size.
            uint32 BytesReceived;
            //! Leftover
            uint32 LeftBytes;
            
            //! State object, can be used to store transient content data.
            void* State;
            
            void* Data;
            
            const class IContentType* ContentType;
        };
        
        static inline bool EnsureContentLength(ContentDesc& contentDesc)
        {
            return contentDesc.FullSize == 0
                || contentDesc.FullSize != contentDesc.BytesReceived;;
        }
    }
}
