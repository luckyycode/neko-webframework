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
//  RequestPoolHandler.cpp
//  Neko SDK
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#include "RequestPoolHandler.h"

namespace Neko::Skylar
{
    static const char* ProtocolH2 = "h2";
    static const char* ProtocolHttp11 = "http/1.1";

    struct Job
    {
        static void Execute(void* ptr)
        {
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

            Net::NetSocketBase outerSocket;
            void* streamData = nullptr; // @todo http/2

            if (not sockets.IsEmpty())
            {
                Tie(outerSocket, streamData) = sockets.front();
                sockets.Pop();
            }

            if (sockets.IsEmpty())
            {
                data->Instance->Event->Reset();
                data->Instance->QueueNotFullEvent.Notify();
            }

            sockets.CriticalSection.Exit();

            if (outerSocket.IsOpen())
            {
                data->Socket = outerSocket;
                data->StreamData = streamData;
            }

            auto socket = data->Socket;

            if (not socket.IsOpen())
            {
                NEKO_DELETE(data->Instance->Allocator, data);
                return;
            }

            // resolve
            if (Net::Endpoint address; socket.GetAddress(address))
            {
                const uint16 port = address.Port;

                // it's a valid tls data, secured
                if (auto it = data->Instance->Ssl->GetTlsData().Find(port); it.IsValid())
                {
                    auto* context = it.value();
                    assert(context != nullptr);
#   if USE_OPENSSL
                    SocketSSL socketSsl(socket, *static_cast<SSL_CTX* >(context));
#   endif
                    if (socketSsl.Handshake())
                    {
                        data->Instance->InstantiateProtocolFor(socketSsl, nullptr);
                    }
                }
                else
                {
                    // use default socket
                    SocketDefault socketDefault(socket);
                    data->Instance->InstantiateProtocolFor(socketDefault, data->StreamData);
                }
            }

            --data->Instance->ConcurrentRequestCount;
            NEKO_DELETE(data->Instance->Allocator, data);
        }

        RequestPoolHandler* Instance;
        Net::NetSocketBase Socket;
        void* StreamData;
    };

    RequestPoolHandler::RequestPoolHandler(Skylar::Server& server, Net::SocketPool& sockets, MT::Event* event)
        : Sockets(sockets)
        , Server(server)
        , Event(event)
        , QueueNotFullEvent(server.QueueNotFullEvent)
        , Settings(server.Settings)
        , Controls(server.Controls)
        , Ssl(server.Ssl)
        , Allocator(server.GetAllocator())
    {
        LogInfo.log("Skylar") << "Request pool handler initializing..";
        ConcurrentRequestCount.Set(0);
    }

    IProtocol* RequestPoolHandler::CreateProto(ISocket& socket, void* stream, IAllocator& allocator) const
    {
        PROFILE_FUNCTION();

        IProtocol* protocol = nullptr;
        if (socket.GetTlsSession() != nullptr)
        {
            auto* session = socket.GetTlsSession();

            char protocolName[12];
            bool result = Ssl->NegotiateProtocol(session, protocolName);

            if (result)
            {
                LogInfo.log("Skylar") << "Protocol negotiated as " << *protocolName;

                if (EqualStrings(protocolName, ProtocolH2))
                {
                    // @todo
                    protocol = nullptr;
                }
                else if (EqualStrings(protocolName, ProtocolHttp11))
                {
                    protocol = NEKO_NEW(allocator, ProtocolHttp)(socket, allocator);
                    protocol->SetSettingsSource(Settings);
                }

                return protocol;
            }

            LogWarning.log("Skylar") << "Tls session data found, but couldn't negotiate a needed protocol";
        }

        protocol = NEKO_NEW(allocator, ProtocolHttp)(socket, allocator);
        protocol->SetSettingsSource(Settings);

        return protocol;
    }

    void RequestPoolHandler::InstantiateProtocolFor(ISocket &socket, void *stream) const
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

    bool RequestPoolHandler::Handle()
    {
        PROFILE_SECTION("Request pool process");

        if (not Controls.Active)
        {
            return false;
        }

        TaskSignal executeSignal = INVALID_HANDLE;

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

            // TaskSignal precondition = lastSignal;
            Async::Run(jobData, &Job::Execute, &executeSignal, INVALID_HANDLE, 0);
        }

        Async::Wait(executeSignal);

        return true;
    }
}


