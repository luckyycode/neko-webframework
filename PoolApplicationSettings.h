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

#include "Engine/Containers/Delegate.h"
#include "Engine/Core/Path.h"

#include "Engine/Network/Http/Request.h"
#include "Engine/Network/Http/ResponseData.h"

#include "ApplicationInitContext.h"

#include <functional>

namespace Neko::Skylar
{
    /** Server settings for each application/pool. */
    struct PoolApplicationSettings
    {
        //! wwwroot
        StaticString<MAX_PATH_LENGTH - 1> RootDirectory;
        //! If empty then system default will be used.
        StaticString<MAX_PATH_LENGTH - 1> TempDirectory;
        
        // Ssl
        
        String CertificateFile;
        String KeyFile;
        
        // Module
        
        String ServerModulePath;
        String ServerModuleUpdatePath;
        
        int32 ModuleIndex;
        
        // Request settings
        
        uint32 RequestMaxSize;
        
        /** Called on early server initialization. */
        std::function< bool(ApplicationInitContext) > OnApplicationInit;
        /** Called on server quit. */
        std::function< void() > OnApplicationExit;
        
        /** Called on incoming request. */
        std::function< int16(Net::Http::RequestData* , Net::Http::ResponseData* ) > OnApplicationRequest;
        /** Called after processing request. */
        std::function< void(Net::Http::ResponseData* ) > OnApplicationPostRequest;
        
        // Ports
        
        uint16 Port;
        //! Secure port, if set secure socket context will be created.
        uint16 TlsPort;
    };
}


