
#pragma once

#include "../../Nova/IController.h"

#include "../TelegramApi.h"

namespace Neko
{
    class TelegramController : public Nova::IController
    {
    public:
        
        NOVA_CONTROLLER(TelegramController);
        
        void Update();
        
    private:
        
        TelegramApi TelegramApi;
    };
}
