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
//  IController.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Engine/Core/Log.h"
#include "Protocol.h"

#include "SessionManager.h"
#include "IController.h"
#include "Router.h"
#include "User.h"

namespace Neko
{
    using namespace Neko::Skylar;
    namespace Nova
    {
        IController::IController(Http::Request& request, Http::Response& response, IAllocator& allocator)
        : Allocator(allocator), QueryParameters(allocator)
        , CookieJar(allocator)
        , HttpRequest(request)
        , HttpResponse(response)
        , Rollback(false)
        , UserManager(nullptr)
        , Session(allocator)
        {
            
        }
        
        IController::~IController()
        {
            
        }

        bool IController::CheckRequest()
        {
            if (not CsrfProtectionEnabled())
            {
                return true;
            }
            
            const auto& sessionStorageType = Options::SessionOptions().StorageType;
            if (sessionStorageType != SessionStorageType::Cookie)
            {
                if (Session.GetId().IsEmpty())
                {
                    LogError.log("Nova") << "Request forgery protection requires a cookie session";
                    return false;
                }
            }
            
            auto csrfTokenIt = HttpRequest.IncomingHeaders.Find("x-csrftoken");
            if (not csrfTokenIt.IsValid())
            {
                csrfTokenIt = HttpRequest.IncomingData.Find("authenticity_token");
            }
            
            if (not csrfTokenIt.IsValid())
            {
                LogWarning.log("Nova") << "CSRF token is empty or not set!";
                return false;
            }
            
            // taken from asp.net core code
            if (not Options::SessionOptions().SuppressXFrameOptionsHeader)
            {
                if (auto frameIt = HttpResponse.Headers.Find("x-frame-options"); !frameIt.IsValid())
                {
                    // http://tools.ietf.org/html/draft-ietf-websec-x-frame-options-10
                    HttpResponse.AddHeader("x-frame-options", "SAMEORIGIN");
                }
            }
            
            return csrfTokenIt.value() == GetAuthToken();
        }

        String IController::GetAuthToken()
        {
            if (Options::SessionOptions().StorageType == SessionStorageType::Cookie)
            {
                const auto& key = Options::SessionOptions().CsrfKey;
                const auto& csrfId = Session.GetValue(key);
                
                if (csrfId.IsEmpty())
                {
                    LogError.log("Nova") << "Csrf ID for session is empty!";
                }
                
                return csrfId;
            }
            else
            {
                auto value = Session.GetId();
                value += Options::SessionOptions().Secret;
                
                String result;
                Nova::Crypt(value, result);
                
                return result;
            }
        }
        
        void IController::SetSession(const class Session& session)
        {
            this->Session = session;
        }
        
        bool IController::AddCookie(const Cookie& cookie)
        {
            if (const auto& name = cookie.Name; name.IsEmpty() or FindFirstOf(*name, ";, \"") != INDEX_NONE)
            {
                LogError.log("Nova") << "Couldn't add cookie with incorrect name: \"" << *name << "\"";
                
                return false;
            }
            
            CookieJar.Push(cookie);
            
            if (auto cookieIt = HttpResponse.Headers.Find("set-cookie"); cookieIt.IsValid())
            {
                HttpResponse.Headers.Erase(cookieIt);
            }
            
            for (auto& cookie : CookieJar)
            {
                const auto& cookieString = cookie.ToString();
                HttpResponse.AddHeader("set-cookie", cookieString);
            }
            return true;
        }
        
        void IController::BadRequest(Http::ObjectResult* result)
        {
            HttpResponse.SetStatusCode(Http::StatusCode::BadRequest);
            if (result != nullptr)
            {
                HttpResponse.SetBodyData(result->Value, result->Size);
            }
        }
        
        void IController::Ok(Http::ObjectResult* result)
        {
            HttpResponse.SetStatusCode(Http::StatusCode::Ok);
            if (result != nullptr)
            {
                HttpResponse.SetBodyData(result->Value, result->Size);
            }
        }
        
        void IController::PhysicalFile(const String& fileName)
        {
            HttpResponse.SetStatusCode(Http::StatusCode::Empty);
            HttpResponse.AddHeader("x-sendfile", fileName);
        }
        
        void IController::Redirect(const String& toUrl)
        {
            HttpResponse.SetStatusCode(Http::StatusCode::MovedTemporarily);
            HttpResponse.AddHeader("Location", *toUrl);
            
            const char* text = "<html><body>Redirected.</body></html>";
            HttpResponse.SetBodyData((uint8* )text, StringLength(text));
        }
    }
}
