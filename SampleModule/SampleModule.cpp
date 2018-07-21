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

#include "../Nova/Options.h"
#include "../Nova/RequestContext.h"
#include "../Nova/IWebApplication.h"
#include "../ApplicationSettings.h"

#include "Controllers/TelegramController.h"
#include "Controllers/HomeController.h"

using namespace Neko;
using namespace Neko::Nova;
using namespace Neko::Skylar;
using namespace Neko::Http;

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

static IWebApplication* Application = nullptr;

extern "C"
{
    /**
     * This is called on early module initialization.
     */
    bool OnApplicationInit(ApplicationInitContext context)
    {
        LogInfo.log("Nova") << "Sample module init";

        const char* rootDirectory = context.RootDirectory;
        SampleModule::DocumentRoot.Assign(rootDirectory);
        
        auto& allocator = *context.AppAllocator;
        Application = NEKO_NEW(allocator, WebApplication) (context);
        
        return Application != nullptr;
    }
    
    /**
     * This is called when requested application has been found.
     */
    int16 OnApplicationRequest(Neko::Http::RequestData* request, Neko::Http::ResponseData * response)
    {
        return Application->ProcessRequest(*request, *response);
    }
    
    /**
     * Called when request has finished.
     */
    void OnApplicationPostRequest(Neko::Http::ResponseData * response)
    {
        Application->CleanupResponseData(*response);
    }
    
    void OnApplicationExit()
    {
        printf("FINAL KEK WAVE\n");
        auto& allocator = Application->GetAllocator();
        NEKO_DELETE(allocator, Application);
        
        Application = nullptr;
    };
}

namespace Neko
{
    String SampleModule::DocumentRoot = String();
}
