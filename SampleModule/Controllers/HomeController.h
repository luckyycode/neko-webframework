
#pragma once

#include "../../Nova/IController.h"

namespace Neko
{
    class HomeController : public Nova::IController
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
