#include "TelegramController.h"

#include "../../../Engine/Network/Network.h"
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Data/JsonSerializer.h"
#include "../../../Engine/Data/MemoryStream.h"
#include "../../../Engine/FS/PlatformFile.h"
#include "../SampleModule.h"

#include <json/json.h>

using json = nlohmann::json;

namespace Neko
{
    template <typename BasicJsonType>
    void from_json(const BasicJsonType& j, Neko::String& out)
    {
        // @todo Oh gosh get rid of it 
        
        std::string aargh = j.template get<std::string>();
        out = String(aargh.c_str());
    }

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
        
        /*
         * incoming data example:
         {
             "update_id": 10000,
             "message": {
                 "date":1441645532,
                 "chat": {
                     "last_name":"Test Lastname",
                     "id":1111111,
                     "first_name":"Test",
                     "username":"Test"
                 },
                 "message_id": 1365,
                 "from":{
                     "last_name":"Test Lastname",
                     "id":1111111,
                     "first_name":"Test",
                     "username":"Test"
                },
                "text": "/start"
             }
         }
         */
        
		// @todo use models instead of manual json reading
		
        if (jsonStringIt.IsValid())
        {
            const auto& jsonString = jsonStringIt.value();
            
            json json = json::parse(*jsonString);
            
            auto& message = json["message"];
            auto& chat = message["chat"];
            long chatId = chat["id"];
            
            const auto textField = message.at("text");
            
            if (textField != nullptr)
            {
                String text = textField.get<String>();
                if (text == "/meow")
                {
                    telegramApi.SendMessage(chatId, "Meow");
                }
                else if (text == "/vzhuh")
                {
                    String message;
                    message = "`\t ∧＿∧  \n";
                    message += "\t( ･ω･｡)つ━☆・*。\n";
                    message += "\t⊂　 ノ 　　　・゜+.\n";
                    message += "\tしーＪ　　　°。+ *´¨)\n";
                    message += "\t\t.· ´¸.·*´¨) ¸.·*¨)\n";
                    message += "\t\t (¸.·´ (¸.·'* ☆\n`";
                    
                    telegramApi.SendMessage(chatId, message, "markdown");
                }
                else if (text == "/kekvzhuh")
                {
                    String message;
                    message = "`\t ∧＿∧  \n";
                    message += "\t( ･ω･｡)つ━☆・*。\n";
                    message += "\t⊂　 ノ 　　　・゜+.\n";
                    message += "\tしーＪ　　　°。+ *´¨)\n";
                    message += "\t\t.· ´¸.·*´¨) ¸.·*¨)\n";
                    message += "\t\t (¸.·´ (¸.·'* ☆\n";
                    message += "\t\t";
                    // pls
                    message += R"(
                                    ___  __    _______   ___  __
                                    |\  \|\  \ |\  ___ \ |\  \|\  \
                                    \ \  \/  /|\ \   __/|\ \  \/  /|_
                                     \ \   ___  \ \  \_|/_\ \   ___  \
                                      \ \  \\ \  \ \  \_|\ \ \  \\ \  \
                                       \ \__\\ \__\ \_______\ \__\\ \__\
                                        \|__| \|__|\|_______|\|__| \|__|`)";
                    
                    
                    telegramApi.SendMessage(chatId, message, "markdown");
                }
                else if (text == "/info")
                {
                    telegramApi.SendMessage(chatId, "Running on _Neko Framework 0.7 linux64_", "markdown");
                }
                else if (text == "/huh")
                {
                    telegramApi.SendSticker(chatId, "CAADAgADXgADuzUFAAF3faBECjOxEQI");
                }
                else if (text == "/random")
                {
                    
                }
            }
        }
        
        HttpResponse.AddHeader("connection", "close");
        
        Ok();
    }
}

