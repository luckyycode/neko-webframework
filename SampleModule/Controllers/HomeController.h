
#pragma once

#include "../../Nova/IController.h"

namespace Neko
{
    class HomeController : public Nova::IController
    {
    public:
        
        HomeController(Http::Request& request, Http::Response& response,
                       IAllocator& allocator);
        
        void Index();
        void Get();
        
        void Login();
        void Logout();
        
        void Check();
    };
}
