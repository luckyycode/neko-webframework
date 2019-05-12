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
#include "Engine/Network/NetSocketBase.h"

#include "Engine/Platform/Platform.h"

#include "Protocol.h"
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
    
    namespace SocketUtils
    {
        /** Creates socket object from a native client socket. */
        static inline ISocket* CreateSocket(Net::NetSocketBase& socket, const Http::RequestData* request, void* stack, bool& secure)
        {
            socket.Init(request->Socket, Net::SocketType::Tcp);
            
            secure = request->TlsSession != nullptr;
            if (secure)
            {
                return new (stack) SocketSSL(socket, *(SSL* )(request->TlsSession));
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
    
    static inline void SerializeResponseData(Http::Response& response, Http::ResponseData& responseData,
            IAllocator& allocator)
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
            OutputData dataStream(responseData.Data, INT_MAX);
            dataStream << outHeaders;
        }
    }
    
    void RequestContext::ProcessRequest(Http::Request& request, Http::Response& response, const RequestMetadata& metadata)
    {
        PROFILE_SECTION("nova process request")
        
        String clearUri;
        
        // Remove uri query parameters if present
        ClearRequestUri(request.Path, clearUri);
        
        // Match route
        
        // route request to controllers
        auto routing = MainRouter.FindRoute(request.Method, clearUri);
        auto& protocol = *metadata.Protocol;

        if (routing.IsValid)
        {
            // route is valid.. controller should be too
            bool executed = ControllerFactory.ExecuteController(routing, protocol, request, response);
            if (not executed)
            {
                LogWarning.log("Nova") << "Couldn't execute a controller.";
            }
        }
        else
        {
            PROFILE_SECTION("Non-mapped route")

            // check if this is a file request
            if (request.Method == Http::Method::Get)
            {
                // build a path
                char path[MAX_PATH_LENGTH];
                CopyString(path, metadata.DocumentRoot);
                CatString(path, *clearUri);
                
                // show directory list
                bool isDirectory = Neko::Platform::DirectoryExists(path);
                if (isDirectory)
                {
                    PROFILE_SECTION("Directory listing")
                    ShowDirectoryList(path, request, response, metadata.Secure, Allocator);
                    
                    protocol.SendResponse(response);
                }
                else
                {
                    PROFILE_SECTION("File send")
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

                response.SetStatusCode(Http::StatusCode::NotFound);
                protocol.SendResponse(response, Http::DEFAULT_RESPONSE_TIME);
            }
        }
    }
    
    RequestMetadata RequestContext::DeserializeRequest(const Http::RequestData& requestData, const Http::ResponseData& responseData, Http::Request& request, Http::Response& response)
    {
        // http version
        uint8 protocolVersion;
        // large socket object
        uint8 stack[sizeof(SocketSSL)];
        
        // socket wrapper
        Net::NetSocketBase netSocket;
        RequestMetadata metadata;
        
        // initialize socket from existing native socket descriptor
        metadata.Socket = SocketUtils::CreateSocket(netSocket, &requestData, &stack, metadata.Secure);

        // read incoming header info
        InputData dataStream(const_cast<void* >(requestData.Data), INT_MAX);
        
        dataStream >> protocolVersion;
        
        const auto version = static_cast<Http::Version>(protocolVersion);
        
        request.ProtocolVersion = version;
        response.ProtocolVersion = version;
        
        switch (version)
        {
            case Http::Version::Http_1:
            {
                request.Deserialize(dataStream);
                metadata.Deserialize(dataStream);
                
                // instantiate protocol
                metadata.Protocol = NEKO_NEW(Allocator, ProtocolHttp)(*metadata.Socket, Allocator);
                
                break;
            }
                
            // @todo
            case Http::Version::Http_2: { break; }
                
            default: { assert(false); break; }
        }
        
        // process request
        Neko::LogInfo.log("Skylar") << "Request ## Http " << static_cast<uint32>(protocolVersion) << " "
            << request.Path << " /" << static_cast<int16>(request.Method);
        
        return metadata;
    }
    
    int16 RequestContext::Execute(Http::RequestData& requestData, Http::ResponseData& responseData)
    {
        PROFILE_FUNCTION()
    
        Http::Request request(Allocator, Version::Unknown); // request
        Http::Response response(Allocator, Version::Unknown); // response
        
        auto metadata = DeserializeRequest(requestData, responseData, request, response);
  
        ProcessRequest(request, response, metadata);
        SocketUtils::DestroySocket(metadata.Socket);
        
        // Post process
        SerializeResponseData(response, responseData, Allocator);
        
        NEKO_DELETE(Allocator, metadata.Protocol);
        
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

