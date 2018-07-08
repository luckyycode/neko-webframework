
#pragma once

#include "../../Mvc/IController.h"

#include "../TelegramApi.h"

namespace Neko
{
    class TelegramController : public Http::IController
    {
    public:
        
        TelegramController(Net::Http::Request& request, Net::Http::Response& response,
                           IAllocator& allocator, const char* name);
        
        void Update();
        
    private:
        
        TelegramApi telegramApi;
    };
}
