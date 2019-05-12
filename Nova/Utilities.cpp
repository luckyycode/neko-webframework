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
//  Utilities.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Utilities.h"

#include "Engine/Utilities/Utilities.h"
#include "Engine/Utilities/Crc32.h"

namespace Neko
{
    namespace Nova
    {
        void Crypt(const String& value, String& result)
        {
            // this is temporary, need to use something more efficient (e.g. sha)
            uint32 crc = Crc32(*value, value.Length());
            result += crc;
            result = Util::BytesToHex((const uint8* )*result, result.Length());
        }
        
        void Uncrypt(const String& value, uint8* data)
        {
            
        }
    }
}
