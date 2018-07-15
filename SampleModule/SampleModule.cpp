//
//  SampleModule.cpp
//  HttpServer
//
//  Created by Neko on 6/29/18.
//

#include "SampleModule.h"

#include "../../Engine/Core/Log.h"
#include "../../Engine/FS/FileSystem.h"
#include "../../Engine/Data/JsonSerializer.h"

#include "../Mvc/Options.h"
#include "../Mvc/RequestContext.h"
#include "../Mvc/IWebApplication.h"
#include "../ApplicationSettings.h"

#include "Controllers/TelegramController.h"
#include "Controllers/HomeController.h"

using namespace Neko;
using namespace Neko::Mvc;

class WebApplication : public IWebApplication
{
public:
    
    WebApplication(const ApplicationInitContext& context)
    : IWebApplication(context)
    {
        LoadSettings();
        
        RegisterControllers();
    }
    
    void RegisterControllers()
    {
        auto& cf = GetContext().GetControllerFactory();
        auto& router = cf.GetRouter();
        
        
        auto* homeContext = cf.CreateControllerContext<HomeController>("home", "/api/home");
        
        homeContext->RouteAction<&HomeController::Index>(router, Net::Http::Method::Get, "index")
            .RouteAction<&HomeController::Get>(router, Net::Http::Method::Get, "get", "[params]")
            .RouteAction<&HomeController::Login>(router, Net::Http::Method::Get, "login")
            .RouteAction<&HomeController::Logout>(router, Net::Http::Method::Get, "logout");
        
        auto* telegramContext = cf.CreateControllerContext<TelegramController>("telegram", "/api/telegram");
        
        telegramContext->RouteAction<&TelegramController::Update>(router, Net::Http::Method::Post, "update");
    }
    
    bool LoadSettings()
    {
        return true;
    }
};

static IWebApplication* Application = nullptr;

extern "C"
{

    /**
     * This is called on early module initialization.
     */
    bool OnApplicationInit(Neko::Http::ApplicationInitContext context)
    {
        GLogInfo.log("Http") << "Sample module init";

        const char* rootDirectory = context.RootDirectory;
        SampleModule::DocumentRoot.Assign(rootDirectory);
        
        auto& allocator = *context.AppAllocator;
        Application = NEKO_NEW(allocator, WebApplication) (context);
        
        return Application != nullptr;
    }
    
    /**
     * This is called when requested application has been found.
     */
    int16 OnApplicationRequest(Neko::Net::Http::RequestData* request, Neko::Net::Http::ResponseData * response)
    {
        return Application->ProcessRequest(*request, *response);
    }
    
    /**
     * Called when request has finished.
     */
    void OnApplicationPostRequest(Neko::Net::Http::ResponseData * response)
    {
        Application->CleanupResponseData(*response);
    }
    
    void OnApplicationExit()
    {
        printf("FINAL KEK WAVE\n");
        auto& allocator = Application->GetAllocator();
        NEKO_DELETE(allocator, Application) ;
        
        Application = nullptr;
    };
}

namespace Neko
{
    String SampleModule::DocumentRoot = String();
}
