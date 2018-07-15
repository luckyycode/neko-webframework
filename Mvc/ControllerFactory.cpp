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
//  ControllerFactory.c
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "../../Engine/Core/Profiler.h"

#include "../Server/IProtocol.h"

#include "Options.h"
#include "UserManager.h"
#include "SessionManager.h"
#include "ControllerFactory.h"
#include "IController.h"

namespace Neko
{
    using namespace Neko::Http;
    namespace Mvc
    {
        ControllerFactory::ControllerFactory(class Router& router, IAllocator& allocator)
        : Allocator(allocator)
        , Router(router)
        , ControllerDispatcher(allocator)
        , UserManager(SessionManager)
        {
        }
       
        /** Creates a new cookie session. */
        static void StoreSessionCookie(IController& controller)
        {
            const int32& cookieLifetime = Options::SessionOptions().Lifetime;
            const String& cookiePath = Options::SessionOptions().CookiePath;
            
            TimeValue value;
            
            value.SetSeconds((int64)cookieLifetime);
            CDateTime expire = CDateTime::UtcNow() + value;
            
            Cookie cookie(Session::GetSessionName(), controller.GetSession().GetId());
            
            cookie.ExpirationDate = expire;
            cookie.Path = cookiePath;
            cookie.Secure = false;
            cookie.HttpOnly = true;
            
            // Sets the path in the session cookie
            controller.AddCookie(cookie);
        }
        
        void ControllerFactory::SetSession(Net::Http::Request& request, IController& controller)
        {
            // session
            if (!controller.IsSessionEnabled())
            {
                return;
            }
            
            auto cookieIt = request.IncomingHeaders.Find("cookie");
            
            Session session;
            
            if (cookieIt.IsValid())
            {
                TArray<Cookie> cookies;
                
                const String& cookieString = cookieIt.value();
                Cookie::ParseCookieString(cookieString, cookies);
                
                const int32 index = cookies.Find([](const Cookie& other) {
                    return other.Name == Session::GetSessionName();
                }); // GET THAT ONE FROM COOKIES
                
                
                if (index != INDEX_NONE)
                {
                    const String& sessionId = cookies[index].Value;
                    // find a session
                    session = SessionManager.FindSession(sessionId);
                }
            }
            
            // set session
            controller.SetSession(session);
        }
        
        void ControllerFactory::ExecuteController(const Routing& routing, IProtocol& protocol, Net::Http::Request& request, Net::Http::Response& response)
        {
            PROFILE_SECTION("controller context execute")
            
            // get controller context
            auto contextIt = ControllerDispatcher.Find(*routing.Controller);
            if (contextIt.IsValid())
            {
                auto* context = contextIt.value();
                
                // create a brand new controller
                IController* controller = context->CreateController(request, response);
                
                assert(controller != nullptr);
                
                // set query params
                controller->SetUrlParameters(routing.Params);
                controller->SetUserManager(&UserManager);
                
                // session
                SetSession(request, *controller);
                
                bool verified = true;
                
                // verify authentication token
                if (Options::SessionOptions().IsCsrfProtectionEnabled && controller->IsCsrfProtectionEnabled() && controller->IsCsrflessAction(*routing.Action))
                {
                    // only for specified methods
                    const auto& method = request.Method;
                    
                    if (method != "get" && method != "head" && method != "options" && method != "trace")
                    {
                        verified = controller->VerifyRequest();
                        if (!verified)
                        {
                            GLogWarning.log("Mvc") << "Incorrect authenticity token!";
                        }
                    }
                }
                
                if (verified)
                {
                    if (controller->IsSessionEnabled())
                    {
                        if (Options::SessionOptions().AutoIdRenewal || !controller->GetSession().IsValid())
                        {
                            // remove the old session
                            SessionManager.Remove(controller->GetSession().Id);

                            // make new session id
                            controller->GetSession().Id = SessionManager.GenerateSessionId();
                            GLogInfo.log("Mvc") << "New session ID: " << *controller->GetSession().Id;
                        }

                        // update csrf data
                        SessionManager.SetCsrfProtectionData(controller->GetSession());
                    }
                    
                    const auto& action = routing.Action;
                    
                    if (controller->PreFilter(action))
                    {
                        // execute controller action
                        context->InvokeAction(*controller, *action);
                        
                        controller->PostFilter();
                        
                        // session store
                        if (controller->IsSessionEnabled())
                        {
                            bool stored = SessionManager.Store(controller->GetSession());
                            if (stored)
                            {
                                StoreSessionCookie(*controller);
                            }
                            else
                            {
                                // shouldn't happen
                                GLogWarning.log("Mvc") << "Couldn't store a requested session!";
                            }
                        }
                    }
                }
                
                // outgoing response is empty, probably controller didn't set anything
                if (response.Status != Net::Http::StatusCode::Empty)
                {
                    // but if something got changed..
                    protocol.SendResponse(response);
                }
                else
                {
                    // response has no data and empty status codes
                    if (!response.HasBodyData())
                    {
                        response.SetStatusCode(Net::Http::StatusCode::InternalServerError);
                    }
                }
                
                context->ReleaseController(controller);
                
                // Session GC
                SessionManager.ClearSessionsCache();
                
                return;
            }
            
            // should never get there
            assert(false);
        }

        void ControllerFactory::Clear()
        {
            if (!this->ControllerDispatcher.IsEmpty())
            {
                for (auto* context : this->ControllerDispatcher)
                {
                    NEKO_DELETE(Allocator, context);
                }
                this->ControllerDispatcher.Clear();
            }
        }
    }
}
