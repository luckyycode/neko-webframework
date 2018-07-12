//
//          *                  *
//             __                *
//           ,db'    *     *
//          ,d8/       *        *    *
//          888
//          `db\       *     *
//            `o`_                    **
//               *                 / )
//             *    /\__/\ *       ( (  *
//           ,-.,-.,)    (.,-.,-.,-.) ).,-.,-.
//          | @|  ={      }= | @|  / / | @|o |
//         _j__j__j_)     `-------/ /__j__j__j_
//          ________(               /___________
//          |  | @| \              || o|O | @|
//          |o |  |,'\       ,   ,'"|  |  |  |  hjw
//          vV\|/vV|`-'\  ,---\   | \Vv\hjwVv\//v
//                     _) )    `. \ /
//                    (__/       ) )
//  _   _      _           _____                                            _
// | \ | | ___| | _____   |  ___| __ __ _ _ __ ___   _____      _____  _ __| | __
// |  \| |/ _ \ |/ / _ \  | |_ | '__/ _` | '_ ` _ \ / _ \ \ /\ / / _ \| '__| |/ /
// | |\  |  __/   < (_) | |  _|| | | (_| | | | | | |  __/\ V  V / (_) | |  |   <
// |_| \_|\___|_|\_\___/  |_|  |_|  \__,_|_| |_| |_|\___| \_/\_/ \___/|_|  |_|\_\
//
//  IController.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "../Server/IProtocol.h"

#include "IController.h"
#include "Router.h"

namespace Neko
{
    using namespace Neko::Http;
    namespace Mvc
    {
        IController::IController(Net::Http::Request& request, Net::Http::Response& response, IAllocator& allocator)
        : Allocator(allocator)
        , Arguments(allocator)
        , HttpRequest(request)
        , HttpResponse(response)
        , Rollback(false)
        {
            
        }
        
        IController::~IController()
        {
            
        }
    
        void IController::BadRequest(Net::Http::ObjectResult* result)
        {
            HttpResponse.SetStatusCode(Net::Http::StatusCode::BadRequest);
            if (result != nullptr)
            {
                HttpResponse.SetBodyData(result->Value, result->Size);
            }
        }
        
        void IController::Ok(Net::Http::ObjectResult* result)
        {
            HttpResponse.SetStatusCode(Net::Http::StatusCode::Ok);
            if (result != nullptr)
            {
                HttpResponse.SetBodyData(result->Value, result->Size);
            }
        }
        
        void IController::PhysicalFile(const String& fileName)
        {
            HttpResponse.SetStatusCode(Net::Http::StatusCode::Empty);
            HttpResponse.AddHeader("x-sendfile", fileName);
        }
        
        void IController::Redirect(const String& toUrl)
        {
            HttpResponse.SetStatusCode(Net::Http::StatusCode::MovedTemporarily);
            HttpResponse.AddHeader("Location", *toUrl);
            
            const char* text = "<html><body>Redirected.</body></html>";
            HttpResponse.SetBodyData((uint8* )text, StringLength(text));
        }
    }
}
