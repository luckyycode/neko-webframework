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
//  TextPlain.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "TextPlain.h"
#include "ContentDesc.h"

namespace Neko
{
    using namespace Neko::Net;
    namespace Skylar
    {
        TextPlain::TextPlain(IAllocator& allocator)
        : IContentType(allocator)
        {
            Name = "text/plain";
        }
        
        bool TextPlain::Parse(const Neko::String& buffer, Http::RequestDataInternal& requestData, ContentDesc* contentDesc) const
        {
            if (buffer.IsEmpty())
            {
                return EnsureContentLength(*contentDesc);
            }
            
            for (int32 pos = 0, end = 0; end != INDEX_NONE; pos = end + 1)
            {
                // find the next param
                end = buffer.Find("&", pos);
                
                if (end == INDEX_NONE && contentDesc->FullSize != contentDesc->BytesReceived)
                {
                    // end of params but we still have something to read
                    
                    // save leftover
                    contentDesc->LeftBytes = buffer.Length() - pos;
                    return true;
                }
                
                // get param value
                int32 delimiter = buffer.Find("=", pos);
                
                const int32 last = (end == INDEX_NONE) ? INT_MAX : end; // hmmm
                
                if (delimiter >= last)
                {
                    // param name
                    Neko::String name = buffer.Mid(pos, (last != INT_MAX) ? end - pos : INT_MAX);
                    
                    // save param
                    requestData.IncomingData.Insert(Neko::Move(name), Neko::String());
                }
                else
                {
                    // param name
                    Neko::String name = buffer.Mid(pos, (last != INT_MAX) ? delimiter - pos : INT_MAX);
                    
                    ++delimiter;
                    
                    // param value
                    Neko::String value = buffer.Mid(delimiter, (end != INDEX_NONE) ? end - delimiter : INT_MAX);
                    
                    // save both
                    requestData.IncomingData.Insert(Neko::Move(name), Neko::Move(value));
                }
            }
            
            return true;
        }
    }
}

