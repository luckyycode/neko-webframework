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
//  ControllerFactory.c
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Engine/Core/Log.h"
#include "Protocol.h"

#include "Engine/Core/Profiler.h"

#include "Options.h"
#include "UserManager.h"
#include "SessionManager.h"
#include "ControllerFactory.h"
#include "IController.h"

namespace Neko::Nova
{
    using namespace Neko::Skylar;
    
    ControllerFactory::ControllerFactory(class IRouter& router, IAllocator& allocator)
        : Allocator(allocator)
        , Router(router)
        , ControllerDispatcher(allocator)
        , SessionManager(allocator)
        , UserManager(SessionManager)
    {
    }
    
    /** Creates a new cookie session. */
    static void StoreSessionCookie(IController& controller)
    {
        const auto& options = Options::SessionOptions();
        
        const int32& lifetime = options.Lifetime;
        const auto& path = options.CookiePath;
        
        TimeSpan value;
        value.SetSeconds(static_cast<int64>(lifetime));
        const DateTime expire = DateTime::UtcNow() + value;
        
        Cookie cookie(Session::GetSessionName(), controller.Session.GetId());
        
        cookie.ExpirationDate = expire;
        cookie.Path = path;
        cookie.Secure = false;
        cookie.HttpOnly = true;
        
        // Sets the path in the session cookie
        controller.AddCookie(cookie);
    }
    
    void ControllerFactory::SetSession(Http::Request& request, IController& controller)
    {
        // session
        if (not controller.IsSessionSupported())
        {
            return;
        }
        
        Session session(Allocator);
        
        if (auto cookieIt = request.IncomingHeaders.Find("cookie"); cookieIt.IsValid())
        {
            TArray<Cookie> cookies(Allocator);
            
            const auto& cookieString = cookieIt.value();
            Cookie::ParseCookieString(cookieString, cookies);
            
            const int32 index = cookies.Find([](const Cookie& other) {
                return other.Name == Session::GetSessionName();
            }); // GET THAT ONE FROM COOKIES


            if (index != INDEX_NONE)
            {
                const auto& sessionId = cookies[index].Value;
                // find a session
                session = SessionManager.FindSession(sessionId);
            }
        }
        
        // set session
        controller.SetSession(session);
    }
    
    bool ControllerFactory::ExecuteController(const Routing& routing, Protocol& protocol, Http::Request& request,
            Http::Response& response)
    {
        PROFILE_SECTION("controller context execute");
        
        // get controller context
        auto* const context = ControllerDispatcher.at(Crc32(*routing.Controller));
        assert(context != nullptr);

        // create a brand new controller
        auto* const controller = context->CreateController(request, response);
        assert(controller != nullptr);

        // set query params
        controller->SetUrlParameters(routing.Parameters);
        controller->SetUserManager(&UserManager);
        
        // session
        SetSession(request, *controller);
        
        bool verified = true;
        
        // verify authentication token
        if (Options::SessionOptions().CsrfProtectionEnabled && controller->CsrfProtectionEnabled())
        {
            // only for specified methods]
            if (request.IsIdempotent())
            {
                verified = controller->CheckRequest();
                if (not verified)
                {
                    LogWarning.log("Nova") << "Incorrect authenticity token!";
                }
            }
        }
        
        if (verified)
        {
            if (controller->IsSessionSupported())
            {
                if (Options::SessionOptions().AutoIdRenewal || not controller->Session.IsValid())
                {
                    // remove the old session
                    SessionManager.Remove(controller->Session.Id);
                    // make new session id
                    controller->Session.Id = SessionManager.NewSessionId();
                }
                
                // update csrf Data
                SessionManager.SetCsrfProtectionData(controller->Session);
            }
            
            
            if (const auto& action = routing.Action; controller->PreFilter(action))
            {
                // execute controller action
                context->InvokeAction(*controller, action);
                controller->PostFilter();
                
                // session store
                if (controller->IsSessionSupported())
                {
                    if (SessionManager.Store(controller->Session))
                    {
                        StoreSessionCookie(*controller);
                    }
                    else
                    {
                        // shouldn't happen
                        LogWarning.log("Nova") << "Couldn't store a requested session!";
                        assert(false);
                    }
                }
            }
        }
        
        // outgoing response is empty, probably controller didn't set anything
        if (response.Status != Http::StatusCode::Empty)
        {
            // but if something got changed..
            protocol.SendResponse(response);
        }
        else
        {
            // response has no Data and empty status codes
            if (not response.HasBodyData())
            {
                response.SetStatusCode(Http::StatusCode::NotImplemented);
            }
        }
        
        context->ReleaseController(*controller);
        // Session cache cleanup
        SessionManager.ClearSessionsCache();

        return true;
    }
    
    void ControllerFactory::Clear()
    {
        if (not this->ControllerDispatcher.IsEmpty())
        {
            for (auto* context : this->ControllerDispatcher)
            {
                NEKO_DELETE(Allocator, context);
            }

            this->ControllerDispatcher.Clear();
        }
    }
}

