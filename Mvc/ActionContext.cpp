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
//  ActionContext.cpp
//  Neko Framework
//
//  Created by Neko on 6/29/18.
//

#include "../../Engine/Core/Log.h"

#include "../../Engine/Network/Http/Response.h"

#include "../../Engine/Network/Http/Extensions/Extensions.h"
#include "../../Engine/Network/NetSocket.h"

#include "../../Engine/Data/Blob.h"
#include "../../Engine/Platform/Platform.h"

#include "../Server/IProtocol.h"
#include "../Server/Http.h"

#include "../SocketSSL.h"
#include "../SocketDefault.h"
#include "../Utils.h"

#include "ActionContext.h"
#include "IController.h"

// temp
#include "../SampleModule/Controllers/FileController.h"
#include "../SampleModule/Controllers/TelegramController.h"

namespace Neko
{
    namespace Http
    {
        // temp
        static void CreateControllers(ControllerFactory& controllerFactory, IAllocator& allocator)
        {
            Http::ControllerContext fileContext(allocator);
            controllerFactory.CreateControllerContext<FileController>(fileContext, "files", "/api/files");
            {
                controllerFactory.RouteAction<FileController, &FileController::Index>(fileContext, Net::Http::Method::Get, "index");
                controllerFactory.RouteAction<FileController, &FileController::Get>(fileContext, Net::Http::Method::Get, "get", "[params]");
                controllerFactory.RouteAction<FileController, &FileController::List>(fileContext, Net::Http::Method::Get, "list");
                
                controllerFactory.Save(fileContext);
            }
            
            Http::ControllerContext telegramContext(allocator);
            controllerFactory.CreateControllerContext<TelegramController>(telegramContext, "telegram", "/api/telegram");
            {
                controllerFactory.RouteAction<TelegramController, &TelegramController::Update>(telegramContext, Net::Http::Method::Post, "update");
                controllerFactory.RouteAction<TelegramController, &TelegramController::Update>(telegramContext, Net::Http::Method::Get, "update");
                
                controllerFactory.Save(telegramContext);
            }
        }
        
        ActionContext::ActionContext()
        : MainRouter(Allocator)
        , ControllerFactory(MainRouter)
        , Allocator()
        {
            CreateControllers(ControllerFactory, Allocator); // temp
        }

        /**
         * Creates socket object from native client socket.
         */
        static ISocket* CreateSocket(Net::INetSocket& socket, Net::Http::RequestData* request, void *addr, bool& secure)
        {
            socket.Init(request->Socket, Net::ESocketType::TCP);
			
            secure = request->TlsSession != nullptr;
			if (secure)
			{
				return new (addr) SocketSSL(socket, (SSL* )request->TlsSession);
			}
			
            return new (addr) SocketDefault(socket);
        }
        
        static void DestroySocket(ISocket* adapter)
        {
            if (adapter != nullptr)
            {
                adapter->~ISocket();
            }
        }
        
        /**
         * Shows directory representation in html.
         */
        static void ShowDirectoryList(const String& documentRoot, const Net::Http::Request& request, Net::Http::Response& response, bool secure, IAllocator& allocator)
        {
            auto fullHost = request.IncomingHeaders.Find("host"); // get host with port (wtf?)
            
            String body(allocator);
            body += R"(
            <head>
                <style>
                    body {
                        background-color: #7E7D7D;
                    }
                </style>
            </head>
            
            <body>
                <p style="color:white"> Exploring: <i><font size=4>)";
            
            body += documentRoot;
            
            body += R"(</font></i>
                <table style="width:50%">
                    <tr>
                        <th style="text-align:left">Name</th>
                        <th style="text-align:left">Last modified</th>
                        <th style="text-align:left">Size</th>
                    </tr><hr>)";
            
            Platform::FileInfo info;
            Platform::FileStatData fileStat;
            
            auto* it = Platform::CreateFileIterator(*documentRoot, allocator);
            
            while (Neko::Platform::GetNextFile(it, &info))
            {
                StaticString<MAX_PATH_LENGTH> fullPath(*documentRoot, "/", info.Filename);
                fileStat = Platform::GetFileData(*fullPath);
                
                if (info.Filename[0] == '.' ||
                    (info.Filename[0] == '.' && info.Filename[1] == '.'))
                {
                    continue;
                }
                
                bool isDirectory = fileStat.bIsDirectory;
                
                // name
                String fileUrl = secure ? "https://" : "http://";
                fileUrl += fullHost.value();
                fileUrl += request.Path;
                fileUrl += "/";
                fileUrl += info.Filename;
                body += "<tr><td><a href=\"";
                body += fileUrl;
                body += "\">";
                body += info.Filename;
                body += "</a></td>";
                
                // last modification
                body += "<td>";
                body += fileStat.ModificationTime.ToString("%d-%m-%y %H:%M");
                body += "</td>";
                
                // size
                if (!isDirectory)
                {
                    body += "<td>";
                    body += fileStat.FileSize / 1024; // @todo StringUtil::BestUnit ?
                    body += " kb.</td></tr>";
                }
                else
                {
                    body += "<td></td></tr>";
                }
            }
            Platform::DestroyFileIterator(it);
            
            body += "</table></p></body>";
            
            response.SetStatusCode(Net::Http::StatusCode::Ok);
            response.SetBodyData((uint8* )*body, body.Length());
        }
        


        int32 ActionContext::Execute(Net::Http::RequestData& requestData, Net::Http::ResponseData& responseData)
        {
            uint8 stack[sizeof(SocketSSL)]; // large socket object
            bool secure;
            Net::INetSocket netSocket;
            ISocket* socket = CreateSocket(netSocket, &requestData, &stack, secure);
            
            const uint8* data = (const uint8* )requestData.Data;
            Net::Http::InputProtocolBlob blob((void* )data, INT_MAX);
            
            // incoming protocol type by request
            IProtocol* protocol = nullptr;
            
            // Read incoming header info
            String documentRoot(Allocator);
            THashMap<String, String> requestCookies(Allocator);
            // http version
            uint8 protocolVersion;
            blob.Read(protocolVersion);
            
            // request
            Net::Http::Request request(Allocator);
    
            const auto version = (Net::Http::Version)protocolVersion;
            request.ProtocolVersion = version;
            
            // response
            Net::Http::Response response(Allocator, version);

            switch (version)
            {
                case Net::Http::Version::Http_1:
                {
                    blob >> request.Host;
                    blob >> request.Path;
                    blob >> request.Method;
                    blob >> documentRoot;
                    
                    blob >> request.IncomingHeaders;
                    blob >> request.IncomingData;
                    
                    blob >> request.IncomingFiles;
                    
                    auto cookieIt = request.IncomingFiles.Find("cookie");
                    if (cookieIt.IsValid())
                    {
                        // @todo
                    }
                    
                    // instantiate protocol
                    protocol = NEKO_NEW(Allocator, ProtocolHttp)(*socket, nullptr, Allocator);
                    
                    break;
                }
                case Net::Http::Version::Http_2:
                {
                    // @todo
                    break;
                }
                default:
                {
                    assert(false);
                    break;
                }
            }
            
            // process request
            
            GLogInfo.log("Http") << "Http " << (uint32)protocolVersion << " " << request.Path << " /" << request.Method;
            
            // Remove uri query parameters if present
            String clearUri(Allocator);
            ClearRequestUri(request.Path, clearUri);
           
            // Match route
            TArray<String> components(Allocator);
            components.Clear();
            
            Router::SplitPath(components, clearUri); // for parsing
            
            // route request to controllers
            auto routing = MainRouter.FindRouting(request.Method, components);
            
            if (routing.Valid)
            {
                // route is valid.. controller should be too
                ControllerFactory.ExecuteController(routing, *protocol, request, response);
            }
            else
            {
                // check if this is a file request
                if (request.Method == "get")
                {
                    // build a path
                    documentRoot.Append(*clearUri);
                    
                    if (documentRoot.Find("/../") == INDEX_NONE)
                    {
                        bool isDirectory = Neko::Platform::DirectoryExists(*documentRoot);
                        if (isDirectory)
                        {
                            ShowDirectoryList(documentRoot, request, response, secure, Allocator);
                            protocol->SendResponse(response);
                        }
                        else
                        {
                            if (Platform::FileExists(*documentRoot))
                            {
                                auto connectionIt = request.IncomingHeaders.Find("connection");
                                
                                if (connectionIt.IsValid())
                                {
                                    response.AddHeader("connection", connectionIt.value());
                                }
                                
                                response.AddHeader("x-sendfile", documentRoot);
                            }
                        }
                    }
                    // sendfile will send needed header
                }
                else
                {
                    GLogWarning.log("Http") << "Request to unmapped url - " << *request.Path;
                    // @todo something
                
                    response.SetStatusCode(Net::Http::StatusCode::NotFound);
                    protocol->SendResponse(response, Net::Http::DEFAULT_RESPONSE_TIME);
                }
            }
            
            DestroySocket(socket);
            
            // Post process
            
            // these will be processed by server after running this app
            auto& outHeaders = response.GetHeaders();
            if (!outHeaders.IsEmpty())
            {
                uint32 size = Net::Http::InputProtocolBlob::GetContainerSize(response.GetHeaders());
                uint8* data = (uint8* )Allocator.Allocate(size * sizeof(uint8));
                
                responseData.Data = (void* )data;
                responseData.Size = size;
                
                Net::Http::OutputProtocolBlob blob(responseData.Data, INT_MAX);
                blob << outHeaders;
            }
            
            NEKO_DELETE(Allocator, protocol);
            
            return APPLICATION_EXIT_SUCCESS;
        }
        
        void ActionContext::CleanupResponseData(void* responseData, uint32 responseSize)
        {
            if (responseData != nullptr && responseSize > 0)
            {
                Allocator.Deallocate(responseData);
            }
        }
    }
}
