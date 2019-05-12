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
//  PoolApplicationSettings.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Core/Path.h"

namespace Neko::FileSystem
{
    class IFileSystem;
}

namespace Neko::Skylar
{
    /** Object used to pass parameters to the application module on init. */
    struct ApplicationInitContext
    {
        const char* RootDirectory = nullptr;
        IAllocator* AppAllocator = nullptr;
        FileSystem::IFileSystem* FileSystem = nullptr;
    };
}

