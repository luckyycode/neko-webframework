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

Mvc::RequestContext* context = nullptr;

static DefaultAllocator baseAllocator;
extern "C"
{
    static void CreateControllers(ControllerFactory& controllerFactory, IAllocator& allocator)
    {
        auto& router = controllerFactory.GetRouter();
        
        ControllerContext fileContext;
        fileContext.Init<FileController>("files", "/api/files");
        {
            fileContext.RouteAction<FileController, &FileController::Index>(router, Net::Http::Method::Get, "index");
            fileContext.RouteAction<FileController, &FileController::Get>(router, Net::Http::Method::Get, "get", "[params]");
            fileContext.RouteAction<FileController, &FileController::List>(router, Net::Http::Method::Get, "list");
        }
        controllerFactory.CreateControllerContext(fileContext);
        
        ControllerContext telegramContext;
        telegramContext.Init<TelegramController>("telegram", "/api/telegram");
        {
            telegramContext.RouteAction<TelegramController, &TelegramController::Update>(router, Net::Http::Method::Post, "update");
        }
        controllerFactory.CreateControllerContext(telegramContext);
    }
    
    /**
     * This is called on early module initialization.
     */
    bool OnApplicationInit(Neko::Http::ApplicationInitDesc desc)
    {
        GLogInfo.log("Http") << "Sample module init";
        
        const char* rootDirectory = desc.RootDirectory;
        SampleModule::DocumentRoot.Assign(rootDirectory);
        
        context = new Mvc::RequestContext(baseAllocator);
        
        auto& controllerFactory = context->GetControllerFactory();
        auto& allocator = context->GetAllocator();
        
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
        delete context;
    };
}

namespace Neko
{
    String SampleModule::DocumentRoot = String(SampleModule::Allocator);
    DefaultAllocator SampleModule::Allocator = DefaultAllocator();
}
