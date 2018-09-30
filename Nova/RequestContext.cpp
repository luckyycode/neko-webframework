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


#include "Engine/FileSystem/FileSystem.h"
#include "Engine/Data/JsonSerializer.h"

#include "Engine/Network/Http/Response.h"
#include "Engine/Network/NetSocket.h"

#include "Engine/Platform/Platform.h"

#include "../Skylar/IProtocol.h"
#include "../Skylar/Http.h"

#include "../Sockets/SocketSSL.h"
#include "../Sockets/SocketDefault.h"
#include "../Utils.h"

#include "Options.h"
#include "RequestContext.h"
#include "IController.h"

namespace Neko::Nova
{
    using namespace Neko::Skylar;
    using namespace Neko::Net;
    
    /**
     * Creates socket object from a native client socket.
     */
    static inline ISocket* CreateSocket(Net::INetSocket& socket, Http::RequestData* request, void* stack, bool& secure)
    {
        socket.Init(request->Socket, Net::ESocketType::TCP);
        
        secure = request->TlsSession != nullptr;
        if (secure)
        {
            return new (stack) SocketSSL(socket, (SSL* )request->TlsSession);
        }
        
        return new (stack) SocketDefault(socket);
    }
    
    static inline void DestroySocket(ISocket* socket)
    {
        if (socket != nullptr)
        {
            // was allocated on a stack
            socket->~ISocket();
        }
    }
    
    RequestContext::RequestContext(IAllocator& allocator, FileSystem::IFileSystem& fileSystem)
    : Allocator(allocator)
    , FileSystem(fileSystem)
    , MainRouter(allocator)
    , ControllerFactory(MainRouter, allocator)
    {
        // defaults
        auto& options = Options::Instance();
        
        options.ConfigureSession([&](SessionOptions& session)
        {
             session.Name = "Neko.CoolCookie";
             session.AutoIdRenewal = false;
             session.CookiePath = "path";
             session.CsrfProtectionEnabled = true;
             session.CsrfKey = "identitysecretkey1337";
             session.Lifetime = 3600;
             session.Secret = "qwerty1234567890";
             session.StorageType = SessionStorageType::Cookie;
        });
        
        LogInfo.log("Nova") << "Nova request context initialized";
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
    
    static inline void WriteResponseData(Http::Response& response, Http::ResponseData& responseData, IAllocator& allocator)
    {
        // these will be processed by server after running this app
        auto& outHeaders = response.GetHeaders();
        if (not outHeaders.IsEmpty())
        {
            auto size = InputData::GetContainerSize(response.GetHeaders());
            auto* data = static_cast<uint8* >(allocator.Allocate(size * sizeof(uint8)));
            
            responseData.Data = data;
            responseData.Size = size;
            // write headers
            OutputData datastream(responseData.Data, INT_MAX);
            datastream << outHeaders;
        }
    }
    
    void RequestContext::ProcessRequest(Skylar::IProtocol& protocol, Http::Request& request, Http::Response& response, const char* documentRoot, const bool secure)
    {
        PROFILE_SECTION("nova process request")
        
        String clearUri;
        
        // Remove uri query parameters if present
        ClearRequestUri(request.Path, clearUri);
        
        // Match route
        
        // route request to controllers
        auto routing = MainRouter.FindRoute(request.Method, clearUri);
        
        if (routing.IsValid)
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
                    
                    // or send a file
                    if (Platform::FileExists(path))
                    {
                        if (auto connectionIt = request.IncomingHeaders.Find("connection"); connectionIt.IsValid())
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
                LogInfo.log("Nova") << "Request to unmapped url - " << *request.Path;
                // @todo something
                
                response.SetStatusCode(Http::StatusCode::NotFound);
                protocol.SendResponse(response, Http::DEFAULT_RESPONSE_TIME);
            }
        }
    }
    
    int16 RequestContext::Execute(Http::RequestData& requestData, Http::ResponseData& responseData)
    {
        PROFILE_FUNCTION()
        
        bool secure;
        // http version
        uint8 protocolVersion;
        // socket wrapper
        Net::INetSocket netSocket;
        // application document root
        char documentRoot[MAX_PATH_LENGTH];
        // large socket object
        uint8 stack[sizeof(SocketSSL)];
        
        // Initialize socket from existing native socket descriptor
        ISocket* socket = CreateSocket(netSocket, &requestData, &stack, secure);
        // incoming protocol type by request
        IProtocol* protocol = nullptr;
        
        // Read incoming header info
        InputData datastream(const_cast<void* >(requestData.Data), INT_MAX);
        
        datastream << protocolVersion;
        
        const auto version = static_cast<Http::Version>(protocolVersion);
        
        Http::Request request(Allocator, version); // request
        Http::Response response(Allocator, version); // response
        
        switch (version)
        {
            case Http::Version::Http_1:
            {
                datastream >> request.Host >> request.Path >> request.Method;
                datastream.ReadString(documentRoot, sizeof(documentRoot));
                datastream >> request.IncomingHeaders >> request.IncomingData >> request.IncomingFiles;
                
                // instantiate protocol
                protocol = NEKO_NEW(Allocator, ProtocolHttp)(*socket, nullptr, Allocator);
                
                break;
            }
                
                // @todo
            case Http::Version::Http_2: { break; }
                
            default: { assert(false); break; }
        }
        
        // process request
        
        LogInfo.log("Skylar") << "Request ## Http " << static_cast<uint32>(protocolVersion) << " " << request.Path << " /" << request.Method;
        
        ProcessRequest(*protocol, request, response, documentRoot, secure);
        DestroySocket(socket);
        
        // Post process
        WriteResponseData(response, responseData, Allocator);
        
        NEKO_DELETE(Allocator, protocol);
        
        return APPLICATION_EXIT_SUCCESS;
    }
    
    void RequestContext::CleanupResponseData(void* responseData, uint32 responseSize)
    {
        if (responseData != nullptr and responseSize > 0)
        {
            Allocator.Deallocate(responseData);
        }
    }
}

