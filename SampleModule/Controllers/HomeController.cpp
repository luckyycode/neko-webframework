#include "HomeController.h"

#include "../../../Engine/Network/Network.h"
#include "../../../Engine/Platform/Platform.h"

#include "../Entities/User.h"
#include "../TelegramApi.h"

namespace Neko
{
    HomeController::HomeController(Net::Http::Request& request, Net::Http::Response& response, IAllocator& allocator)
    : IController(request, response, allocator)
    {
        
    }
    
    void HomeController::Index()
    {
        const char* aaa = GetUserManager().IsUserAuthenticated(GetSession()) ? "Perhaps" : "No";
        Net::Http::ObjectResult result
        {
            .Value = (uint8* )aaa,
            .Size = StringLength(aaa)
        };
        
        Ok(&result);
        //PhysicalFile("index.html");
    }
    
    void HomeController::Get()
    {
        const auto& params = this->GetUrlParameters();
        
        if (params.GetSize() == 0)
        {
            Redirect("/api/home/index");
            return;
        }
        
        PhysicalFile(params[0]);
    }

    void HomeController::Login()
    {
        User user;
        user.UserName = "neko";
        
        auto& session = GetSession();
        
        GetUserManager().UserLogin(user, session);
        
        const char* aaa = *user.UserName;
        Net::Http::ObjectResult result
        {
            .Value = (uint8* )aaa,
            .Size = StringLength(aaa)
        };
        Ok(&result);
    }
    
    void HomeController::Logout()
    {
        GetUserManager().UserLogout(GetSession());
    }
}

