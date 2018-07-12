//
//  SampleModule.cpp
//  HttpServer
//
//  Created by Neko on 6/29/18.
//

#include "SampleModule.h"

#include "../../Engine/Core/Log.h"

#include "../Mvc/RequestContext.h"
#include "../ApplicationSettings.h"

#include "Controllers/TelegramController.h"
#include "Controllers/FileController.h"

using namespace Neko;
using namespace Neko::Mvc;

static Mvc::RequestContext* context = nullptr;

extern "C"
{
    static void CreateControllers(ControllerFactory& cf, IAllocator& allocator)
    {
        auto& router = cf.GetRouter();
        
        auto* fileContext = cf.CreateControllerContext<FileController>("files", "/api/files");
        {
            fileContext->RouteAction<&FileController::Index>(router, Net::Http::Method::Get, "index");
            fileContext->RouteAction<&FileController::Get>(router, Net::Http::Method::Get, "get", "[params]");
            fileContext->RouteAction<&FileController::List>(router, Net::Http::Method::Get, "list");
        }
        
        auto* telegramContext = cf.CreateControllerContext<TelegramController>("telegram", "/api/telegram");
        {
            telegramContext->RouteAction<&TelegramController::Update>(router, Net::Http::Method::Post, "update");
        }
    }
    
    /**
     * This is called on early module initialization.
     */
    bool OnApplicationInit(Neko::Http::ApplicationInitDesc desc)
    {
        GLogInfo.log("Http") << "Sample module init";
        
        auto& allocator = *desc.AppAllocator;
        
        const char* rootDirectory = desc.RootDirectory;
        SampleModule::DocumentRoot.Assign(rootDirectory);
        
        context = NEKO_NEW(allocator, Mvc::RequestContext) (allocator);
        
        auto& controllerFactory = context->GetControllerFactory();
        CreateControllers(controllerFactory, allocator); // temp
        
        return context != nullptr;
    }
    
    /**
     * This is called when requested application has been found.
     */
    int OnApplicationRequest(Neko::Net::Http::RequestData* request, Neko::Net::Http::ResponseData * response)
    {
        return context->Execute(*request, *response);
    }
    
    /**
     * Called when request has finished.
     */
    void OnApplicationPostRequest(Neko::Net::Http::ResponseData * response)
    {
        context->CleanupResponseData(response->Data, response->Size);
    }
    
    void OnApplicationExit()
    {
        printf("FINAL KEK WAVE\n");
        auto& allocator = context->GetAllocator();
        NEKO_DELETE(allocator, context) ;
        
        context = nullptr;
    };
}

namespace Neko
{
    String SampleModule::DocumentRoot = String();
}
