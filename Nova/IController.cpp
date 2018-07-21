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
//  IController.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "../Skylar/IProtocol.h"

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
        : Allocator(allocator)
        , QueryParameters(allocator)
        , CookieJar(allocator)
        , HttpRequest(request)
        , HttpResponse(response)
        , Rollback(false)
        , UserManager(nullptr)
        {
            
        }
        
        IController::~IController()
        {
            
        }

        bool IController::VerifyRequest()
        {
            if (!IsCsrfProtectionEnabled())
            {
                return true;
            }
            
            const String& sessionStorageType = Options::SessionOptions().StorageType;
            if (sessionStorageType != "cookie")
            {
                if (Session.GetId().IsEmpty())
                {
                    GLogError.log("Nova") << "Request forgery protection requires a cookie session";
                    return false;
                }
            }
            
            auto csrfTokenIt = HttpRequest.IncomingHeaders.Find("x-csrftoken");
            if (!csrfTokenIt.IsValid())
            {
                csrfTokenIt = HttpRequest.IncomingData.Find("authenticity_token");
            }
            
            if (!csrfTokenIt.IsValid())
            {
                GLogWarning.log("Nova") << "CSRF token is empty!";
                return false;
            }
            
            if (!Options::SessionOptions().SuppressXFrameOptionsHeader)
            {
                auto frameIt = HttpResponse.Headers.Find("x-frame-options");
                if (!frameIt.IsValid())
                {
                    // http://tools.ietf.org/html/draft-ietf-websec-x-frame-options-10
                    HttpResponse.AddHeader("x-frame-options", "SAMEORIGIN");
                }
            }
            
            return csrfTokenIt.value() == GetAuthToken();
        }

        String IController::GetAuthToken()
        {
            if (Options::SessionOptions().StorageType == "cookie")
            {
                const String& key = Options::SessionOptions().CsrfKey;
                const String& csrfId = Session.GetValue(key);
                
                if (csrfId.IsEmpty())
                {
                    GLogError.log("Nova") << "Csrf ID for session is empty!";
                }
                
                return csrfId;
            }
            else
            {
                String value = Session.GetId();
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
        
        bool IController::AddCookie(Cookie cookie)
        {
            const String& name = cookie.Name;
            
            if (name.IsEmpty() || FindFirstOf(*name, ";, \"") != INDEX_NONE)
            {
                GLogError.log("Nova") << "Couldn't add cookie with incorrect name: \"" << *name << "\"";
                
                return false;
            }
            
            CookieJar.Push(cookie);
            
            auto cookieIt = HttpResponse.Headers.Find("Set-Cookie");
            if (cookieIt.IsValid())
            {
                HttpResponse.Headers.Erase(cookieIt);
            }
            
            for (auto& cookie : CookieJar)
            {
                const auto& cookieString = cookie.ToString();
                HttpResponse.AddHeader("Set-Cookie", cookieString);
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
