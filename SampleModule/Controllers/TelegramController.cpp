#include "TelegramController.h"

#include "../../../Engine/Network/Network.h"
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Data/JsonSerializer.h"
#include "../../../Engine/Data/MemoryStream.h"
#include "../../../Engine/FS/PlatformFile.h"
#include "../SampleModule.h"

#include "../Models/UpdateModel.h"

#include <json/json.h>

using json = nlohmann::json;

namespace Neko
{
    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j, Neko::String& out)
    {
        std::string basicType = j.template get<std::string>();
        out = String(basicType.c_str());
    }

    TelegramController::TelegramController(Http::Request& request, Http::Response& response, IAllocator& allocator)
    : IController(request, response, allocator)
    , TelegramApi(GetAllocator())
    {
        TelegramApi.SetBotToken("613232204:AAG3XMqvr7aLDrZmuGcV9O9AgQycU4zCtoo");
    }
    
    void TelegramController::Update()
    {
        const auto& data = HttpRequest.IncomingData;
        const auto& jsonStringIt = data.Find("jsonData");

		// @todo use models instead of manual json reading

        HttpResponse.AddHeader("connection", "close");
        
        Ok();
    }
}

