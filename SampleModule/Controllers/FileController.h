
#pragma once

#include "../../Mvc/IController.h"

namespace Neko
{
    class FileController : public Http::IController
    {
    public:
        
        FileController(Net::Http::Request& request, Net::Http::Response& response,
                       IAllocator& allocator, const char* name);
        
        void Index();
        
        void Get();
        
        void List();
        
        void Random();
    };
}
