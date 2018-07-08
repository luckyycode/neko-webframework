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
//  ApplicationSettings.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../Engine/Containers/Delegate.h"

#include "../Engine/Network/Http/Request.h"
#include "../Engine/Network/Http/ResponseData.h"

#include <functional>

namespace Neko
{
    namespace Http
    {
        /** Object used to pass parameters to the application module on init. */
        struct ApplicationInitDesc
        {
            const char* RootDirectory = nullptr;
        };
        
        /** Server settings for each application. */
        struct ApplicationSettings
        {
            // Request settings
            uint32 RequestMaxSize;
            
            String RootDirectory;
            String TempDirectory;
            
            // Ports
            uint32 Port;
            uint32 TlsPort;
            
            // Module
            int32 ModuleIndex;
            
            String ServerModule;
            String ServerModuleUpdate;
            
            // Ssl
            String CertificateFile;
            String KeyFile;
            
            /** Called on early server initialization. */
            std::function< bool(ApplicationInitDesc) > OnApplicationInit;
            /** Called on server quit. */
            std::function< void() > OnApplicationExit;
            
            /** Called on incoming request. */
            std::function< int(Net::Http::RequestData* , Net::Http::ResponseData* ) > OnApplicationRequest;
            /** Called after processing request. */
            std::function< void(Net::Http::ResponseData* ) > OnApplicationPostRequest;
        };
    }
}

