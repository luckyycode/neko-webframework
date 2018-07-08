#include "TelegramController.h"

#include "../../../Engine/Network/Network.h"
#include "../../../Engine/Platform/Platform.h"

#include <json/json.h>

namespace Neko
{
    TelegramController::TelegramController(Net::Http::Request& request, Net::Http::Response& response, IAllocator& allocator, const char* name)
    : IController(request, response, allocator, name)
    , telegramApi(GetAllocator())
    {
        telegramApi.SetBotToken("613232204:AAG3XMqvr7aLDrZmuGcV9O9AgQycU4zCtoo");
    }
    
    void TelegramController::Update()
    {
        const auto& data = HttpRequest.IncomingData;
        const auto& jsonStringIt = data.Find("jsonData");
        
        if (jsonStringIt.IsValid())
        {
            const auto& jsonString = jsonStringIt.value();
            
            nlohmann::json json = nlohmann::json::parse(*jsonString);
            
            printf("/update/: \n %s \n", *jsonString);
            
            telegramApi.SendMessage(-293406528, "Nya");
        }
        
        HttpResponse.AddHeader("connection", "close");
        
        Ok();
    }
}

