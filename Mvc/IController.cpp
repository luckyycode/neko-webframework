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

#include "../Server/IProtocol.h"

#include "SessionManager.h"
#include "IController.h"
#include "Router.h"
#include "User.h"

#define SESSION_USER_NAME   "loginUserName"

namespace Neko
{
    using namespace Neko::Http;
    namespace Mvc
    {
        IController::IController(Net::Http::Request& request, Net::Http::Response& response, IAllocator& allocator)
        : Allocator(allocator)
        , Arguments(allocator)
        , CookieJar(allocator)
        , HttpRequest(request)
        , HttpResponse(response)
        , Rollback(false)
        {
            
        }
        
        IController::~IController()
        {
            
        }
    
        bool IController::IsUserAuthenticated()
        {
            auto keyIt = Session.Find(SESSION_USER_NAME);
            return keyIt.IsValid();
        }
        
        bool IController::UserLogin(const IUser& user)
        {
            if (user.GetIdentityKey().IsEmpty())
            {
                GLogWarning.log("Mvc") << "User identity key is empty!";
                return false;
            }
            
            if (IsUserAuthenticated())
            {
                GLogInfo.log("Mvc") << "User is already authenticated!";
            }
            
            GetSession().Insert(SESSION_USER_NAME, user.GetIdentityKey());
            
            GLogInfo.log("Mvc") << "User logged in!";
            
            return true;
        }
        
        void IController::UserLogout()
        {
            auto& session = GetSession();
            
            const auto keyIt = session.Find(SESSION_USER_NAME);
            if (keyIt.IsValid())
            {
                GLogInfo.log("Mvc") << "User logged out.";
                session.Erase(keyIt);
            }
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
                if (GetSession().GetId().IsEmpty())
                {
                    GLogError.log("Mvc") << "Request forgery protection requires a cookie session";
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
                GLogWarning.log("Mvc") << "CSRF token is empty!";
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
                const String& csrfId = GetSession().GetValue(key);
                
                if (csrfId.IsEmpty())
                {
                    GLogError.log("Mvc") << "Csrf ID for session is empty!";
                }
                
                return csrfId;
            }
            else
            {
                String value = GetSession().GetId();
                value += Options::SessionOptions().Secret;
                
                String result;
                Mvc::Crypt(value, result);
                
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
                GLogError.log("Mvc") << "Couldn't add cookie with incorrect name: \"" << *name << "\"";
                
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
        
        void IController::BadRequest(Net::Http::ObjectResult* result)
        {
            HttpResponse.SetStatusCode(Net::Http::StatusCode::BadRequest);
            if (result != nullptr)
            {
                HttpResponse.SetBodyData(result->Value, result->Size);
            }
        }
        
        void IController::Ok(Net::Http::ObjectResult* result)
        {
            HttpResponse.SetStatusCode(Net::Http::StatusCode::Ok);
            if (result != nullptr)
            {
                HttpResponse.SetBodyData(result->Value, result->Size);
            }
        }
        
        void IController::PhysicalFile(const String& fileName)
        {
            HttpResponse.SetStatusCode(Net::Http::StatusCode::Empty);
            HttpResponse.AddHeader("x-sendfile", fileName);
        }
        
        void IController::Redirect(const String& toUrl)
        {
            HttpResponse.SetStatusCode(Net::Http::StatusCode::MovedTemporarily);
            HttpResponse.AddHeader("Location", *toUrl);
            
            const char* text = "<html><body>Redirected.</body></html>";
            HttpResponse.SetBodyData((uint8* )text, StringLength(text));
        }
    }
}
