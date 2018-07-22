
#pragma once

#include "../../Nova/IController.h"

namespace Neko
{
    class HomeController : public Nova::IController
    {
    public:
        
        NOVA_CONTROLLER(HomeController);
        
        void Index();
        void Get();
        
        void Login();
        void Logout();
        
        void Check();
    };
}
