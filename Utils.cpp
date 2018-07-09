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
//  Utils.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Utils.h"

#include "../Engine/Utilities/Templates.h"
#include "../Engine/Utilities/Utilities.h"
#include "../Engine/Core/Log.h"

namespace Neko
{
    namespace Http
    {
        String GetMimeByFileName(const String& fileName, const THashMap<String, String>& mimes)
        {
            // find extension start
            const int32 extensionPos = fileName.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromStart); // @todo possible optimization? FromEnd
            
            if (extensionPos != INDEX_NONE)
            {
                String extension = fileName.Mid(extensionPos + 1 /* skip dot */).ToLower() ;
                const auto mimeIt = mimes.Find(extension);
                
                if (mimeIt.IsValid())
                {
                    return mimeIt.value();
                }
            }
            
            return String(DefaultMimeType);
        }
        
        void GetIncomingQueryVars(THashMap<String, String>& incomingData, const String& uri, IAllocator& allocator)
        {
            // according to doc only POST and PUT requests should check content-type/content-length info
            // but well...
            
            // Parse URI query params
            
            const int32 start = uri.Find("?");
            
            if (start != INDEX_NONE)
            {
                const int32 finish = uri.Find("#");
                
                // # is missing or found and must be at the end..
                if (finish == INDEX_NONE || (finish != INDEX_NONE && finish > start))
                {
                    for (int32 paramCur = start + 1, paramEnd = 0; paramEnd != INDEX_NONE; paramCur = paramEnd + 1)
                    {
                        paramEnd = uri.Find("&", ESearchCase::IgnoreCase, ESearchDir::FromStart, paramCur);
                        
                        if (finish != INDEX_NONE && paramEnd > finish)
                        {
                            paramEnd = INDEX_NONE;
                        }
                        
                        int32 delimiter = uri.Find("=", ESearchCase::IgnoreCase, ESearchDir::FromStart, paramCur);
                        
                        if (delimiter >= paramEnd)
                        {
                            String name(allocator);
                            Util::DecodeUrl(uri.Mid(paramCur, INDEX_NONE != paramEnd ? paramEnd - paramCur : INT_MAX), name);
                            
                            incomingData.Insert(Neko::Move(name), Neko::String(allocator));
                        }
                        else
                        {
                            String name(allocator);
                            Util::DecodeUrl(uri.Mid(paramCur, delimiter - paramCur), name);
                            
                            ++delimiter;
                            
                            String value(allocator);
                            Util::DecodeUrl(uri.Mid(delimiter, paramEnd != INDEX_NONE ? paramEnd - delimiter : INT_MAX), value);
                            
                            incomingData.Insert(Neko::Move(name), Neko::Move(value));
                        }
                    }
                }
            }
        }
        
        void ClearRequestUri(const String& path, String& clean)
        {
            const int32 index = FindFirstOf(*path, "?#");
            
            Util::DecodeUrl(index == INDEX_NONE ? path : path.Mid(0, index), clean);
        }
        
    }
}

