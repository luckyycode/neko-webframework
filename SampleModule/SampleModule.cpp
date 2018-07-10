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

extern "C"
{
    static void CreateControllers(ControllerFactory& cf, IAllocator& allocator)
    {
        ControllerContext fileContext(allocator);
        cf.CreateControllerContext<FileController>(fileContext, "files", "/api/files");
        {
            cf.RouteAction<FileController, &FileController::Index>(fileContext, Net::Http::Method::Get, "index");
            cf.RouteAction<FileController, &FileController::Get>(fileContext, Net::Http::Method::Get, "get", "[params]");
            cf.RouteAction<FileController, &FileController::List>(fileContext, Net::Http::Method::Get, "list");
            
            cf.Save(fileContext);
        }
        
        ControllerContext telegramContext(allocator);
        cf.CreateControllerContext<TelegramController>(telegramContext, "telegram", "/api/telegram");
        {
            cf.RouteAction<TelegramController, &TelegramController::Update>(telegramContext, Net::Http::Method::Post, "update");
            cf.RouteAction<TelegramController, &TelegramController::Update>(telegramContext, Net::Http::Method::Get, "update");
            
            cf.Save(telegramContext);
        }
    }
    
    /**
     * This is called on early module initialization.
     */
    bool OnApplicationInit(Neko::Http::ApplicationInitDesc desc)
    {
        GLogInfo.log("Http") << "Sample module init";
        
        const char* rootDirectory = desc.RootDirectory;
        SampleModule::DocumentRoot.Assign(rootDirectory);
        
        context = new Mvc::RequestContext(GetDefaultAllocator());
        
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
