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
//  PoolApplicationSettings.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/Delegate.h"
#include "Engine/Core/Path.h"

#include "Engine/Network/Http/Request.h"
#include "Engine/Network/Http/ResponseData.h"

#include <functional>

namespace Neko
{
    namespace FileSystem
    {
        class IFileSystem;
    }
    namespace Skylar
    {
        /** Object used to pass parameters to the application module on init. */
        struct ApplicationInitContext
        {
            const char* RootDirectory = nullptr;
            IAllocator* AppAllocator = nullptr;
            FileSystem::IFileSystem* FileSystem = nullptr;
        };
        
        /** Server settings for each application/pool. */
        struct PoolApplicationSettings
        {
            // Request settings
            
            uint32 RequestMaxSize;
            
            //! wwwroot
            StaticString<MAX_PATH_LENGTH> RootDirectory;
            //! If empty then system default will be used.
            StaticString<MAX_PATH_LENGTH> TempDirectory;
            
            // Ports
            
            uint16 Port;
            //! Secure port, if set then secure socket context will be created.
            uint16 TlsPort;
            
            // Module
            
            int32 ModuleIndex;
            
            String ServerModulePath;
            String ServerModuleUpdatePath;
            
            // Ssl
            
            String CertificateFile;
            String KeyFile;
            
            /** Called on early server initialization. */
            std::function< bool(ApplicationInitContext) > OnApplicationInit;
            /** Called on server quit. */
            std::function< void() > OnApplicationExit;
            
            /** Called on incoming request. */
            std::function< int16(Net::Http::RequestData* , Net::Http::ResponseData* ) > OnApplicationRequest;
            /** Called after processing request. */
            std::function< void(Net::Http::ResponseData* ) > OnApplicationPostRequest;
        };
    }
}

