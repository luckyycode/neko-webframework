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
//  RequestTask.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Server.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Debug.h"
#include "Engine/Core/Asynchrony.h"
#include "Engine/Core/Profiler.h"
#include "Engine/Mt/Task.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Platform/SocketList.h"
#include "Engine/Network/SocketQueue.h"
#include "Engine/FileSystem/PlatformFile.h"

#include "Server.h"
#include "Http.h"
#include "ISsl.h"
#include "IProtocol.h"

#include "../Sockets/SocketSSL.h"
#include "../Sockets/SocketDefault.h"

namespace Neko::Skylar
{
    // Task for processing incoming requests. @see Server::ProcessWorkerThreads
    class RequestTask : public MT::Task
    {
    public:
        
        RequestTask(Server& server, IAllocator& allocator, Net::SocketsQueue& sockets, int volatile* threadRequestCounter)
        : MT::Task(allocator)
        , Sockets(sockets)
        , ThreadRequestCounter(threadRequestCounter)
        , QueueNotFullEvent(server.QueueNotFullEvent)
        , Settings(server.Settings)
        , Controls(server.Controls)
        , ThreadsWorkingCount(server.ThreadsWorkingCount)
        , Ssl(server.Ssl)
        , Allocator(allocator)
        {
        }
        
        IProtocol* CreateProto(ISocket& socket, void* stream, IAllocator& allocator) const
        {
            PROFILE_FUNCTION()
            
            IProtocol* protocol = nullptr;
            
            if (socket.GetTlsSession() != nullptr)
            {
                auto session = socket.GetTlsSession();
                
                char protocolName[12];
                bool result = Ssl->NegotiateProtocol(session, protocolName);
                
                if (result != false)
                {
                    LogInfo.log("Skylar") << "Protocol negotiated as " << *protocolName;
                    
                    if (EqualStrings(protocolName, "h2"))
                    {
                        // @todo
                        protocol = nullptr;
                    }
                    else if (EqualStrings(protocolName, "http/1.1"))
                    {
                        protocol = NEKO_NEW(allocator, ProtocolHttp)(socket, allocator);
                        protocol->SetSettingsSource(Settings);
                    }
                    
                    return protocol;
                }
                
                //LogWarning.log("Skylar") << "Tls session data found, but couldn't negotiate the needed protocol";
            }
            
            protocol = NEKO_NEW(allocator, ProtocolHttp)(socket, allocator);
            protocol->SetSettingsSource(Settings);
            
            return protocol;
        }
        
        void ThreadRequestProc(ISocket& socket, Net::SocketsQueue& sockets, void* stream) const
        {
            auto* protocol = CreateProto(socket, stream, Allocator);
            
            if (protocol != nullptr)
            {
                // Check if switching protocol
                for (IProtocol* result = nullptr; ;)
                {
                    // This may return a new instance if switching protocols
                    result = protocol->Process();
                    // ..so check
                    if (protocol == result)
                    {
                        break;
                    }
                    
                    NEKO_DELETE(Allocator, protocol);
                    protocol = result;
                }
                
                protocol->Close();
            }
            
            NEKO_DELETE(Allocator, protocol);
        }
        
        virtual int32 DoTask() override
        {
            SetThreadDefaultAllocator(Allocator);
            
            while (true)
            {
                PROFILE_SECTION("Server request process");
                
                Net::INetSocket socket;
                void* streamData = nullptr; // @todo http/2
                
                Asynchrony::Await(ThreadRequestCounter);
                
                if (not Controls.Active)
                {
                    break;
                }
                
                Asynchrony::TaskData task;
                Asynchrony::LambdaTask storage;
                {
                    MT::SpinLock lock(Sockets.Mutex);
                    Asynchrony::From([&]()
                    {
                        // get socket and stream data
                        if (not Sockets.IsEmpty())
                        {
                            Tie(socket, streamData) = Sockets.front();
                             
                            Sockets.Pop();
                        }
                         
                        if (Sockets.IsEmpty())
                        {
                            *ThreadRequestCounter = 0; // Reset
                            QueueNotFullEvent.Trigger();
                        }
                    }, &storage, &task, nullptr);
                    
                    TaskCounter counter = 0;
                    Asynchrony::Run(&task, 1, &counter);
                    Asynchrony::Await(&counter);
                }
                if (not socket.IsOpen())
                {
                    continue;
                }
                
                ++ThreadsWorkingCount;
                
                // resolve
                if (Net::NetAddress address; socket.GetAddress(address))
                {
                    const uint16 port = address.Port;
                    
                    // it's a valid tls data, secured
                    if (auto it = Ssl->GetTlsData().Find(port); it.IsValid())
                    {
                        auto* context = it.value();
                        assert(context != nullptr);
#   if USE_OPENSSL
                        SocketSSL socketSsl(socket, static_cast<SSL_CTX* >(context));
#   endif
                        if (socketSsl.Handshake())
                        {
                            ThreadRequestProc(socketSsl, Sockets, nullptr);
                        }
                    }
                    else
                    {
                        // use default socket
                        SocketDefault socketDefault(socket);
                        
                        ThreadRequestProc(socketDefault, Sockets, streamData);
                    }
                }
                
                --ThreadsWorkingCount;
            }
            
            return EXIT_SUCCESS;
        }
        
    private:
        
        CycleManager& Controls;
        ServerSharedSettings& Settings;
        
        Net::SocketsQueue& Sockets;
        
        int volatile* ThreadRequestCounter;
        MT::Event& QueueNotFullEvent;
        
        IAllocator& Allocator;
        
        ISsl* Ssl;
        
        ThreadSafeCounter& ThreadsWorkingCount;
    };
    
}


