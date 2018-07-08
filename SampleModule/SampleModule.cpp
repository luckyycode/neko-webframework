//
//  SampleModule.cpp
//  HttpServer
//
//  Created by Neko on 6/29/18.
//

#include "SampleModule.h"

#include "../../Engine/Core/Log.h"

#include "../Mvc/ActionContext.h"

#include "Controllers/TelegramController.h"
#include "Controllers/FileController.h"

using namespace Neko;

Http::ActionContext* context = nullptr;

extern "C"
{
    /**
     * This is called on early module initialization.
     */
    bool OnApplicationInit(const char* rootDirectory)
    {
        GLogInfo.log("Http") << "Core module init";
        
        SampleModule::DocumentRoot.Assign(rootDirectory);
        
        context = new Http::ActionContext();
        
        return context != nullptr;
    }
    
    /**
     * This is called when requested application has been found.
     */
    int OnApplicationCall(Neko::Net::Http::RequestData* request, Neko::Net::Http::ResponseData * response)
    {
        return context->Execute(*request, *response);
    }
    
    /**
     * Called when request has finished.
     */
    void OnApplicationClear(Neko::Net::Http::ResponseData * response)
    {
        context->CleanupResponseData(response->Data, response->Size);
    }
    
    void OnApplicationExit(const char* rootDirectory)
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
