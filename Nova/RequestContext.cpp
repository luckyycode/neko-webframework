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

#include "Engine/Core/Log.h"
#include "Engine/Core/Profiler.h"


#include "Engine/FS/FileSystem.h"
#include "Engine/Data/JsonSerializer.h"

#include "Engine/Network/Http/Response.h"
#include "Engine/Network/NetSocket.h"

#include "Engine/Platform/Platform.h"

#include "../Skylar/IProtocol.h"
#include "../Skylar/Http.h"

#include "../SocketSSL.h"
#include "../SocketDefault.h"
#include "../Utils.h"

#include "Options.h"
#include "RequestContext.h"
#include "IController.h"

namespace Neko
{
    using namespace Neko::Skylar;
    using namespace Neko::Net;
    namespace Nova
    {
        /**
         * Creates socket object from native client socket.
         */
        static ISocket* CreateSocket(Net::INetSocket& socket, Http::RequestData* request, void* addr, bool& secure)
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
        
        RequestContext::RequestContext(IAllocator& allocator, FS::FileSystem& fileSystem)
        : Allocator(allocator)
        , FileSystem(fileSystem)
        , MainRouter(allocator)
        , ControllerFactory(MainRouter, allocator)
        {
            // defaults
            auto& options = Options::Instance();
            
            options.Configure([&](Options& options)
            {
                auto& session = options.Session;
                
                session.Name = "Neko.CoolCookie";
                session.AutoIdRenewal = false;
                session.CookiePath = "cookiePath";
                session.IsCsrfProtectionEnabled = true;
                session.CsrfKey = "identitysecretkey1337";
                session.Lifetime = 3600;
                session.Secret = "qwerty1234567890";
                session.GcProbability = 5;
                session.MaxGcLifetime = 1000;
                session.StorageType = "cookie";
            });
        }
        
        RequestContext::~RequestContext()
        {
            ControllerFactory.Clear();
        }
        
        struct AppSettings
        {
            template <typename TFunc>
            void LoadFrom(TFunc func)
            {
                
            }
        };
 
        static void WriteResponseData(Http::Response& response, Http::ResponseData& responseData, IAllocator& allocator)
        {
            // these will be processed by server after running this app
            auto& outHeaders = response.GetHeaders();
            if (!outHeaders.IsEmpty())
            {
                uint32 size = InputBlob::GetContainerSize(response.GetHeaders());
                uint8* data = static_cast<uint8* >(allocator.Allocate(size * sizeof(uint8)));
                
                responseData.Data = data;
                responseData.Size = size;
                // write headers
                OutputBlob blob(responseData.Data, INT_MAX);
                blob << outHeaders;
            }
        }
        
        void RequestContext::ProcessRequest(Skylar::IProtocol& protocol, Http::Request& request, Http::Response& response, const char* documentRoot, const bool secure)
        {
            PROFILE_SECTION("mvc process request")
            
            String clearUri(Allocator);
            TArray< String > components(Allocator);
            THashMap< String, String > requestCookies(Allocator);
            
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
                    char path[MAX_PATH_LENGTH];
                    CopyString(path, documentRoot);
                    CatString(path, *clearUri);
                    
                    // show directory list
                    bool isDirectory = Neko::Platform::DirectoryExists(path);
                    if (isDirectory)
                    {
                        PROFILE_SECTION("directory listing")
                        
                        ShowDirectoryList(path, request, response, secure, Allocator);
                        
                        protocol.SendResponse(response);
                    }
                    else
                    {
                        PROFILE_SECTION("file send")
                        
                        // or send file
                        if (Platform::FileExists(path))
                        {
                            auto connectionIt = request.IncomingHeaders.Find("connection");
                            
                            if (connectionIt.IsValid())
                            {
                                response.AddHeader("connection", connectionIt.value());
                            }
                            
                            response.AddHeader("x-sendfile", path);
                        }
                    }
                    // sendfile will send needed header
                }
                else
                {
                    GLogInfo.log("Skylar") << "Request to unmapped url - " << *request.Path;
                    // @todo something
                    
                    response.SetStatusCode(Http::StatusCode::NotFound);
                    protocol.SendResponse(response, Http::DEFAULT_RESPONSE_TIME);
                }
            }
        }
        
        int32 RequestContext::Execute(Http::RequestData& requestData, Http::ResponseData& responseData)
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
            InputBlob blob((void* )requestData.Data, INT_MAX);
            
            char documentRoot[MAX_PATH_LENGTH];
            // http version
            uint8 protocolVersion;
            blob.Read(protocolVersion);
            
            const auto version = static_cast<Http::Version>(protocolVersion);
            
            Http::Request request(Allocator, version); // request
            Http::Response response(Allocator, version); // response

            switch (version)
            {
                case Http::Version::Http_1:
                {
                    blob >> request.Host >> request.Path >> request.Method;
                    blob.ReadString(documentRoot, sizeof(documentRoot));
                    blob >> request.IncomingHeaders >> request.IncomingData >> request.IncomingFiles;
                    
                    // instantiate protocol
                    protocol = NEKO_NEW(Allocator, ProtocolHttp)(*socket, nullptr, Allocator);
                    
                    break;
                }
                    
                // @todo
                case Http::Version::Http_2: { break; }
                    
                default: { assert(false); break; }
            }
            
            // process request
            
            GLogInfo.log("Skylar") << "Request ## Http " << static_cast<uint32>(protocolVersion) << " " << request.Path << " /" << request.Method;
            
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
