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
//  IWebApplication.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../ApplicationSettings.h"

namespace Neko
{
    namespace Nova
    {
        /// Interface for main application.
        class IWebApplication
        {
        public:
            
            IWebApplication(const Skylar::ApplicationInitContext& context)
            : Context(*context.AppAllocator, *context.FileSystem)
            {
            }
            
            inline int16 ProcessRequest(Http::RequestData& request, Http::ResponseData& response)
            {
                return Context.Execute(request, response);
            }
            
            inline void CleanupResponseData(Http::ResponseData& data)
            {
                Context.CleanupResponseData(data.Data, data.Size);
            }
            
            NEKO_FORCE_INLINE RequestContext& GetContext() { return this->Context; }
            
            NEKO_FORCE_INLINE IAllocator& GetAllocator() { return Context.GetAllocator(); }
            
        private:
            
            //! Request context.
            RequestContext Context;
        };
    }
}
