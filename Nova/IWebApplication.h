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
//  IWebApplication.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../PoolApplicationSettings.h"

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

// Basic macro for application initialization
#define NOVA_APPLICATION_ENTRY(classname) \
    static IWebApplication* Application = nullptr; \
    extern "C" \
    { \
        bool OnApplicationInit(ApplicationInitContext context) \
        { \
            LogInfo.log("Nova") << "Application module init: " << #classname; \
            auto& allocator = *context.AppAllocator; \
            Application = NEKO_NEW(allocator, classname) (context); \
            return Application != nullptr; \
        } \
        int16 OnApplicationRequest(Neko::Http::RequestData* request, Neko::Http::ResponseData * response) \
        { \
            return Application->ProcessRequest(*request, *response); \
        } \
        void OnApplicationPostRequest(Neko::Http::ResponseData * response) \
        { \
            Application->CleanupResponseData(*response); \
        } \
        void OnApplicationExit() \
        { \
            printf("FINAL KEK WAVE\n"); \
            auto& allocator = Application->GetAllocator(); \
            NEKO_DELETE(allocator, Application); \
            Application = nullptr; \
        };\
    }\
