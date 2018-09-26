//
//  SampleModule.cpp
//  HttpServer
//
//  Created by Neko on 6/29/18.
//

#include "SampleModule.h"

#include "../../Engine/Core/Log.h"
#include "../../Engine/FileSystem/FileSystem.h"
#include "../../Engine/Data/JsonSerializer.h"

#include "../Nova/Options.h"
#include "../Nova/RequestContext.h"
#include "../Nova/IWebApplication.h"
#include "../PoolApplicationSettings.h"

#include "Controllers/TelegramController.h"
#include "Controllers/HomeController.h"

using namespace Neko;
using namespace Neko::Nova;
using namespace Neko::Net::Http;

class WebApplication : public IWebApplication
{
public:
    
    explicit WebApplication(const ApplicationInitContext& context)
    : IWebApplication(context)
    {
        LoadSettings();
        
        auto& cf = GetContext().GetControllerFactory();
        
        cf.CreateContext<HomeController>("home", "/api")
            .RouteAction<&HomeController::Index>(Method::Get, "index")
            .RouteAction<&HomeController::Get>(Method::Get, "get", "[params]")
            .RouteAction<&HomeController::Login>(Method::Get, "login")
            .RouteAction<&HomeController::Logout>(Method::Get, "logout");
        
        cf.CreateContext<TelegramController>("telegram", "/api")
            .RouteAction<&TelegramController::Update>(Method::Post, "update");
    }
    
    bool LoadSettings()
    {
        return true;
    }
};


NOVA_APPLICATION_ENTRY(WebApplication)
