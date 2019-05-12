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
//  RequestContext.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "ControllerFactory.h"
#include "Router.h"
#include "RequestMetadata.h"

namespace Neko::Nova
{
    /** Provides higher level side for processing requests (e.g. routes, controllers) */
    class RequestContext
    {
    public:
        RequestContext(IAllocator& allocator, FileSystem::IFileSystem& fileSystem);
        
        ~RequestContext();
        
        /** Executes the given request with the higher level logic. */
        int16 Execute(Http::RequestData& requestData, Http::ResponseData& responseData);
        
        /** Called after request execution, clean up for a response data. */
        void CleanupResponseData(void* responseData, uint32 responseSize);
        
    private:
        RequestMetadata DeserializeRequest(const Http::RequestData& requestData, const Http::ResponseData& responseData, Http::Request& request, Http::Response& response);
        
        void ProcessRequest(Http::Request& request, Http::Response& response, const RequestMetadata& metadata);
        
    public:
        /** Returns the instance of controller factory. */
        NEKO_FORCE_INLINE ControllerFactory& GetControllerFactory() { return this->ControllerFactory; }
        
        NEKO_FORCE_INLINE IAllocator& GetAllocator() { return Allocator; }
        
    private:
        //! Url router.
        Router MainRouter;
        
        //! Main controller factory.
        class ControllerFactory ControllerFactory;
        
        IAllocator& Allocator;
        FileSystem::IFileSystem& FileSystem;
    };
}

