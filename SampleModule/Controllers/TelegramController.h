
#pragma once

#include "../../Nova/IController.h"

#include "../TelegramApi.h"

namespace Neko
{
    class TelegramController : public Nova::IController
    {
    public:
        
        TelegramController(Http::Request& request, Http::Response& response, IAllocator& allocator);
        
        void Update();
        
    private:
        
        TelegramApi TelegramApi;
    };
}
