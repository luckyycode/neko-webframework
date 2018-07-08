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
//  ServerApplicationSettings.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../Engine/Containers/Delegate.h"

#include "../Engine/Network/Http/Request.h"
#include "../Engine/Network/Http/ResponseData.h"

#include <functional>

#define APPLICATION_EXIT_SUCCESS EXIT_SUCCESS
#define APPLICATION_EXIT_FAILURE EXIT_FAILURE

namespace Neko
{
    namespace Http
    {
        /** Server settings for each application. */
        struct ServerApplicationSettings
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
            
            // Tls
            String CertFile;
            String KeyFile;
            String CrlFile;
            String ChainFile;
            String DhFile;
            String StaplingFile;
            
            std::function< int(Net::Http::RequestData* , Net::Http::ResponseData* ) > OnApplicationCall;
            std::function< void(Net::Http::ResponseData* ) > OnApplicationClear;
            std::function< bool(const char* ) > OnApplicationInit;
            std::function< void(const char* ) > OnApplicationExit;
        };
    }
}

