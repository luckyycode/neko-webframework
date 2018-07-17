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
//  RequestContext.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "ControllerFactory.h"
#include "Router.h"

namespace Neko
{
    namespace Nova
    {
        /// Provides higher level side for processing requests (e.g. routes, controllers)
        class RequestContext
        {
        public:
            
            RequestContext(IAllocator& allocator, FS::FileSystem& fileSystem);
            
            ~RequestContext();
            
            /**
             * Executes the given request with the higher level logic.
             */
            int32 Execute(Net::Http::RequestData& requestData, Net::Http::ResponseData& responseData);
            
            /**
             * Called after request execution, clean up for a response data.
             */
            void CleanupResponseData(void* responseData, uint32 responseSize);
            
        private:
            
            void ProcessRequest(Skylar::IProtocol& protocol, Net::Http::Request& request, Net::Http::Response& response, String& documentRoot, const bool secure);
            
        public:
            
            /**
             * Returns the instance of controller factory.
             */
            NEKO_FORCE_INLINE ControllerFactory& GetControllerFactory() { return this->ControllerFactory; }
            
            NEKO_FORCE_INLINE IAllocator& GetAllocator() { return Allocator; }
            
        private:
            
            //! Url router.
            Router MainRouter;
            
            //! Main controller factory.
            ControllerFactory ControllerFactory;
            
            IAllocator& Allocator;
            FS::FileSystem& FileSystem;
        };
    }
}
