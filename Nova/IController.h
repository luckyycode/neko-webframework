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
//  IController.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/NekoString.h"
#include "Engine/Network/Http/Response.h"
#include "Engine/Network/Http/Request.h"

#include "../Utils.h"

#include "UserManager.h"
#include "Utilities.h"
#include "Cookie.h"
#include "Session.h"
#include "Router.h"

#include "Model.h"

// helper
#define NOVA_CONTROLLER(x)  x(Http::Request& request, Http::Response& response, IAllocator& allocator);

namespace Neko
{
    using namespace Neko::Skylar;
    namespace Nova
    {
        /// Controller interface.
        class IController
        {
        public:
       
            /** Instance of a controller. */
            IController(Http::Request& request, Http::Response& response, IAllocator& allocator);
            
            virtual ~IController();

        public:
            // middleware additions
            
            /** Called before request action execution. */
            virtual bool PreFilter(const char* action) { return true; };
            
            /** Called after request action execution. */
            virtual void PostFilter() { };

        public:
            // csrf/authentication
            
            virtual bool CsrfProtectionEnabled() const { return true; }
            
            String GetAuthToken();
            
            bool CheckRequest();
            
        public:
            // session
            
            virtual bool IsSessionSupported() const { return true; }
            
            /** Sets the request session data. */
            void SetSession(const Session& session);
            
            /** Adds cookie to this controller's cookie jar. */
            bool AddCookie(const class Cookie& cookie);
            
        public:
            
            /** This controller's response reference. */
            const NEKO_FORCE_INLINE Http::Response& GetHttpResponse() { return HttpResponse; }
            
            /** Parameters from url / arguments tagged as [param] or [params] */
            const NEKO_FORCE_INLINE TArray<String>& GetUrlParameters() const { return this->QueryParameters; }

            /** Sets this controller arguments for action. */
            void SetUrlParameters(const TArray<String>& arguments) { this->QueryParameters = arguments; }
            
            /** Requests transaction rollback. */
            void RollbackTransaction() { this->Rollback = true; }
            
            bool NEKO_FORCE_INLINE IsRollbackRequested() const { return Rollback; }
            
        public:
            // responses
            
            void Ok(Http::ObjectResult* result = nullptr);
            
            void BadRequest(Http::ObjectResult* result = nullptr);
            
            /** Sets x-sendfile header */
            void PhysicalFile(const String& fileName);
            
            /** Redirects to the given url and sets 302 status code. */
            void Redirect(const String& toUrl);
            
        public:
            
            IAllocator& GetAllocator() { return Allocator; }
            
            NEKO_FORCE_INLINE UserManager& GetUserManager()
            {
                assert(UserManager != nullptr);
                return *UserManager;
            }
            
        private:
            friend class ControllerFactory;
            
            void SetUserManager(UserManager* userManager) { this->UserManager = userManager; }
            
            bool Rollback;
            
            //! Incoming parameters (url arguments)
            TArray<String> QueryParameters;
            
            IAllocator& Allocator;
            
            CookieJar CookieJar;
            
            UserManager* UserManager;
            
        public:
            
            Session Session;
            
            // @note Lifetime of these is the same as the lifetime of this controller.
            
            Http::Response& HttpResponse;
            
            Http::Request& HttpRequest;
        };
    }
}
