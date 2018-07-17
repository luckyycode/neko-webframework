
#pragma once

#include "../../Nova/IController.h"

#include "../TelegramApi.h"

namespace Neko
{
    class TelegramController : public Nova::IController
    {
    public:
        
        TelegramController(Net::Http::Request& request, Net::Http::Response& response, IAllocator& allocator);
        
        void Update();
        
    private:
        
        TelegramApi TelegramApi;
    };
}
