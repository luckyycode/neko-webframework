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
//  IController.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../../Engine/Utilities/NekoString.h"
#include "../../Engine/Network/Http/Response.h"
#include "../../Engine/Network/Http/Request.h"

#include "../Utils.h"

#include "UserManager.h"
#include "Utilities.h"
#include "Cookie.h"
#include "Session.h"
#include "Router.h"

#include "Model.h"

namespace Neko
{
    using namespace Neko::Skylar;
    namespace Nova
    {
        /// Controller interface.
        class IController
        {
        public:
       
            /**
             * Instance of a controller.
             *
             * @param path  Controller url (e.g. can be /files or /api/files).
             */
            IController(Net::Http::Request& request, Net::Http::Response& response, IAllocator& allocator);
            
            virtual ~IController();

        public:
            
            /** Called before request action execution. */
            virtual bool PreFilter(const String& action) { return true; };
            
            /** Called after request action execution. */
            virtual void PostFilter() { };

            virtual bool IsSessionEnabled() const { return true; }
            
            virtual bool IsCsrfProtectionEnabled() const { return true; }
            
            String GetAuthToken();
            
            bool IsCsrflessAction(const char* action) const { return true; }
            
            bool VerifyRequest();
            
            void SetSession(const Session& session);
            void SetUserManager(UserManager* userManager) { this->UserManager = userManager; }
            
            bool AddCookie(Cookie cookie);
            
            /** This controller's response reference. */
            const NEKO_FORCE_INLINE Net::Http::Response& GetHttpResponse() { return HttpResponse; }
            
            /** Parameters from url / arguments tagged as [param] or [params] */
            const NEKO_FORCE_INLINE TArray<String>& GetUrlParameters() const { return this->QueryParameters; }

            /** Sets this controller arguments for action. */
            void SetUrlParameters(const TArray<String>& arguments)
            {
                this->QueryParameters = arguments;
            }
            
            /** Requests transaction rollback. */
            void RollbackTransaction()
            {
                this->Rollback = true;
            }
            
            bool NEKO_FORCE_INLINE IsRollbackRequested() const { return Rollback; }
            
        public:
            
            // responses
            
            void Ok(Net::Http::ObjectResult* result = nullptr);
            
            void BadRequest(Net::Http::ObjectResult* result = nullptr);
            
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
            
            bool Rollback;
            
            //! Incoming parameters (url arguments)
            TArray<String> QueryParameters;
            
            IAllocator& Allocator;
            
            CookieJar CookieJar;
            
            UserManager* UserManager;
            
        public:
            
            Session Session;
            
            // @note Lifetime of these is the same as the lifetime of this controller.
            
            Net::Http::Response& HttpResponse;
            
            Net::Http::Request& HttpRequest;
        };
    }
}
