
#pragma once

#include "../../Mvc/IController.h"

namespace Neko
{
    class HomeController : public Mvc::IController
    {
    public:
        
        HomeController(Net::Http::Request& request, Net::Http::Response& response,
                       IAllocator& allocator);
        
        void Index();
        void Get();
        
        void Login();
        void Logout();
        
        void Check();
    };
}
