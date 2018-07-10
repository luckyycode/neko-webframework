#include "FileController.h"

#include "../../../Engine/Network/Network.h"
#include "../../../Engine/Platform/Platform.h"

#include "../TelegramApi.h"


namespace Neko
{
    FileController::FileController(Net::Http::Request& request, Net::Http::Response& response, IAllocator& allocator, const char* name)
    : IController(request, response, allocator, name)
    {
        
    }
    
    void FileController::Index()
    {
        PhysicalFile("index.html");
    }
    
    void FileController::Get()
    {
        const auto& params = this->GetUrlParameters();
        
        if (params.GetSize() == 0)
        {
            Redirect("/api/files/index");
            return;
        }
        
        PhysicalFile(params[0]);
    }
    
    void FileController::List()
    {
        Ok();
    }
    
    void FileController::Random()
    {
    }
}

