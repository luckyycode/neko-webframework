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
//  RequestContext.cpp
//  Neko Framework
//
//  Created by Neko on 6/29/18.
//

#include "../../Engine/Core/Log.h"
#include "../../Engine/Core/Profiler.h"


#include "../../Engine/Data/LifoAllocator.h"

#include "../../Engine/Network/Http/Response.h"

#include "../../Engine/Network/Http/Extensions/Extensions.h"
#include "../../Engine/Network/NetSocket.h"

#include "../../Engine/Platform/Platform.h"

#include "../Server/IProtocol.h"
#include "../Server/Http.h"

#include "../SocketSSL.h"
#include "../SocketDefault.h"
#include "../Utils.h"

#include "RequestContext.h"
#include "IController.h"

namespace Neko
{
    using namespace Neko::Http;
    namespace Mvc
    {
        /**
         * Creates socket object from native client socket.
         */
        static ISocket* CreateSocket(Net::INetSocket& socket, Net::Http::RequestData* request, void* addr, bool& secure)
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
                // was allocated on a stack
                adapter->~ISocket();
            }
        }
        
        RequestContext::RequestContext(IAllocator& allocator)
        : Allocator(allocator)
        , MainRouter(allocator)
        , ControllerFactory(MainRouter, allocator)
        {
        }
        
        RequestContext::~RequestContext()
        {
            ControllerFactory.Clear();
        }
        
        static void WriteResponseData(Net::Http::Response& response, Net::Http::ResponseData& responseData, IAllocator& allocator)
        {
            // these will be processed by server after running this app
            auto& outHeaders = response.GetHeaders();
            if (!outHeaders.IsEmpty())
            {
                uint32 size = Net::Http::InputProtocolBlob::GetContainerSize(response.GetHeaders());
                uint8* data = static_cast<uint8* >(allocator.Allocate(size * sizeof(uint8)));
                
                responseData.Data = data;
                responseData.Size = size;
                // write headers
                Net::Http::OutputProtocolBlob blob(responseData.Data, INT_MAX);
                blob << outHeaders;
            }
        }
        
        void RequestContext::ProcessRequest(Http::IProtocol& protocol, Net::Http::Request& request, Net::Http::Response& response, String& documentRoot, const bool secure)
        {
            PROFILE_SECTION("mvc process request")
            
            String clearUri(Allocator);
            TArray<String> components(Allocator);
            THashMap<String, String> requestCookies(Allocator);
            
            auto cookieIt = request.IncomingHeaders.Find("cookie");
            if (cookieIt.IsValid())
            {
                // @todo
            }
            
            // Remove uri query parameters if present
            ClearRequestUri(request.Path, clearUri);
            
            // Match route
            Router::SplitPath(components, clearUri); // for parsing
            
            // route request to controllers
            auto routing = MainRouter.FindRouting(request.Method, components);
            
            if (routing.Valid)
            {
                // route is valid.. controller should be too
                ControllerFactory.ExecuteController(routing, protocol, request, response);
            }
            else
            {
                PROFILE_SECTION("non-mapped route")
                
                // check if this is a file request
                if (request.Method == "get")
                {
                    // build a path
                    documentRoot.Append(*clearUri);
                    
                    if (documentRoot.Find("/../") == INDEX_NONE)
                    {
                        // show directory list
                        bool isDirectory = Neko::Platform::DirectoryExists(*documentRoot);
                        if (isDirectory)
                        {
                            PROFILE_SECTION("directory listing")
                            
                            ShowDirectoryList(documentRoot, request, response, secure, Allocator);
                            
                            protocol.SendResponse(response);
                        }
                        else
                        {
                            PROFILE_SECTION("file send")
                            
                            // or send file
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
                    GLogInfo.log("Http") << "Request to unmapped url - " << *request.Path;
                    // @todo something
                    
                    response.SetStatusCode(Net::Http::StatusCode::NotFound);
                    protocol.SendResponse(response, Net::Http::DEFAULT_RESPONSE_TIME);
                }
            }
        }
        
        int32 RequestContext::Execute(Net::Http::RequestData& requestData, Net::Http::ResponseData& responseData)
        {
            PROFILE_FUNCTION()
            
            // Initialize socket from existing native socket descriptor
            
            uint8 stack[sizeof(SocketSSL)]; // large socket object
            bool secure;
            Net::INetSocket netSocket;
            ISocket* socket = CreateSocket(netSocket, &requestData, &stack, secure);
            
            // incoming protocol type by request
            IProtocol* protocol = nullptr;
            
            // Read incoming header info
            Net::Http::InputProtocolBlob blob((void* )requestData.Data, INT_MAX);
            
            String documentRoot;
            // http version
            uint8 protocolVersion;
            blob.Read(protocolVersion);
            
            // request
            Net::Http::Request request(Allocator);
    
            const auto version = static_cast<Net::Http::Version>(protocolVersion);
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
            
            GLogInfo.log("Http") << "Request ## Http " << static_cast<uint32>(protocolVersion) << " " << request.Path << " /" << request.Method;
            
            ProcessRequest(*protocol, request, response, documentRoot, secure);
            
            DestroySocket(socket);
            
            // Post process
            WriteResponseData(response, responseData, Allocator);
            
            NEKO_DELETE(Allocator, protocol);
            
            return APPLICATION_EXIT_SUCCESS;
        }
        
        void RequestContext::CleanupResponseData(void* responseData, uint32 responseSize)
        {
            if (responseData != nullptr && responseSize > 0)
            {
                Allocator.Deallocate(responseData);
            }
        }
    }
}
