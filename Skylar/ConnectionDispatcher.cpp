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
//  ConnectionDispatcher.cpp
//  Neko SDK
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#include "ConnectionDispatcher.h"
#include "ConnectionContext.h"
#include "ConnectionManager.h"
#include "ListenOptions.h"

namespace Neko::Skylar
{
    static const char* ProtocolH2 = "h2";
    static const char* ProtocolHttp11 = "http/1.1";

    Sync::ThreadSafeCounter64 ConnectionDispatcher::LastConnectionId = LONG_MIN;
    
    struct HttpConnectionMiddleware : MiddlewareDelegate
    {
        HttpConnectionMiddleware(MiddlewareDelegate& next)
            : Next(next)
        {
            this->Bind<HttpConnectionMiddleware, &HttpConnectionMiddleware::Execute>(this);
        }
        
        void Execute(ConnectionContext& context)
        {
            Next.Invoke(context);
        }
        
    private:
        MiddlewareDelegate& Next;
    };
    
    struct Job
    {
        static void Execute(void* ptr)
        {
            int64 connectionId = ConnectionDispatcher::LastConnectionId.Increment();
            
            auto* data = static_cast<Job* >(ptr);
            auto& sockets = data->Instance->Sockets;

            sockets.CriticalSection.Enter();
            
            if (sockets.IsEmpty())
            {
                sockets.CriticalSection.Exit();
                NEKO_DELETE(data->Instance->Allocator, data);
                
                return;
            }
            ++data->Instance->ConcurrentRequestCount;
            
            NetSocketBase socket;
            void* streamData = nullptr;
            if (not sockets.IsEmpty())
            {
                auto socketConnection = sockets.front();
                
                socket = socketConnection.Socket;
                streamData = socketConnection.Data;
                
                sockets.Pop();
            }

            if (sockets.IsEmpty())
            {
                data->Instance->Event->Reset();
                data->Instance->QueueNotFullEvent.Notify();
            }
            
            sockets.CriticalSection.Exit();

            if (not socket.IsOpen())
            {
                NEKO_DELETE(data->Instance->Allocator, data);
                LogWarning("Skylar") << "Connection with id " << (uint64) connectionId << " refused.";
                
                return;
            }

            // resolve
            if (Net::Endpoint address; socket.GetAddress(address))
            {
                TransportConnection transport(address, connectionId);

                const uint16 port = address.Port;
                bool handled = false;

                // todo process middleware

                // it's a valid tls Data, secured
                if (auto portIt = data->Instance->Ssl->GetTlsData().Find(port); portIt.IsValid())
                {
                    auto* context = portIt.value();
                    assert(context != nullptr);

                    SocketSSL socketSsl(socket, *static_cast<SSL_CTX* >(context));
                    SkylarConnection connection(transport, socketSsl);
                    
                    if (socketSsl.Handshake())
                    {
                        handled = data->Instance->InstantiateProtocolFromConnection(connection);
                    }
                }
                else
                {
                    // use default socket
                    SocketDefault socketDefault(socket);
                    SkylarConnection connection(transport, socketDefault);
                    
                    connection.StreamData = streamData;
                    
                    handled = data->Instance->InstantiateProtocolFromConnection(connection);
                }
                
                if (not handled)
                {
                    LogError("Skylar") << "Unhandled exception while processing " << (uint64) connectionId << ".";
                }
            }

            --data->Instance->ConcurrentRequestCount;
            NEKO_DELETE(data->Instance->Allocator, data);
        }

        ConnectionDispatcher* Instance;
    };

    ConnectionDispatcher::ConnectionDispatcher(Skylar::Server& server, Net::SocketPool& sockets,
        Sync::Event* event, MiddlewareDelegate& delegate)
        : Sockets(sockets)
        , Server(server)
        , Event(event)
        , ConnectionManager(server.GetAllocator())
        , QueueNotFullEvent(server.QueueNotFullEvent)
        , Modules(server.Modules)
        , Controls(server.Controls)
        , Ssl(server.Binder.Ssl)
        , Allocator(server.GetAllocator())
        , Delegate(delegate), Options(server.GetAllocator())
        , Timer()
    {
        LogInfo("Skylar") << "Connection dispatcher initializing..";
        ConcurrentRequestCount.Set(0);
        
        Options.Build();
    }

    ConnectionDispatcher::~ConnectionDispatcher()
    {
        Options.Destroy();
    }

    IProtocol* ConnectionDispatcher::CreateProto(SkylarConnection& connection, IAllocator& allocator) const
    {
        PROFILE_FUNCTION();
        
        auto& socket = connection.SocketConnection;

        IProtocol* protocol = nullptr;
        auto* session = socket.GetTlsSession();
        
        if (session != nullptr)
        {
            char protocolName[12];
            bool result = Ssl->NegotiateProtocol(session, protocolName);

            if (result)
            {
                LogInfo("Skylar") << "Connection protocol negotiated as " << *protocolName;

                if (EqualStrings(protocolName, ProtocolH2))
                {
                    // @todo http/2
                    protocol = nullptr;
                }
                else if (EqualStrings(protocolName, ProtocolHttp11))
                {
                    protocol = NEKO_NEW(allocator, ProtocolHttp)(socket, Options, allocator);
                    protocol->SetSettingsSource(Modules);
                }

                return protocol;
            }

            LogWarning("Skylar") << "Tls session Data found, but couldn't negotiate a needed protocol.";
        }

        protocol = NEKO_NEW(allocator, ProtocolHttp)(socket, Options, allocator);
        protocol->SetSettingsSource(Modules);

        return protocol;
    }

    bool ConnectionDispatcher::InstantiateProtocolFromConnection(SkylarConnection &connection) const
    {
        TimeSpan start = Timer.GetAsyncTime(), end; // profiling
        auto* protocol = CreateProto(connection, Allocator);
        
        bool handled = true;
        if (protocol != nullptr)
        {
            // Check if switching protocol
            for (IProtocol* result = nullptr; ;)
            {
                // This may return a new instance if switching protocols
                result = protocol->Process();

                // but also may return null in case of fail
                if (result == nullptr)
                {
                    handled = false;

                    break;
                }
                
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
        else
        {
            handled = false;
        }

        end = Timer.GetAsyncTime();
        
        LogInfo("Skylar") << "Request completed in " << (end - start).GetMilliSeconds() << "ms";
        NEKO_DELETE(Allocator, protocol);
        
        return handled;
    }

    bool ConnectionDispatcher::Handle()
    {
        PROFILE_SECTION("Connection dispatcher");
        if (not Controls.Active)
        {
            return false;
        }

        Async::TaskCounter executeCounter = Async::INVALID_HANDLE;
        while (true)
        {
            Sockets.CriticalSection.Enter();

            if (Sockets.IsEmpty())
            {
                Sockets.CriticalSection.Exit();
                break;
            }

            Sockets.CriticalSection.Exit();

            Job* jobData = NEKO_NEW(Allocator, Job);
            jobData->Instance = this;

            // TaskCounter PreCondition = lastSignal;
            Async::Run(jobData, &Job::Execute, &executeCounter);
        }

        Async::Wait(executeCounter);

        return true;
    }
    
    void ConnectionDispatcher::Complete()
    {
    }
}


