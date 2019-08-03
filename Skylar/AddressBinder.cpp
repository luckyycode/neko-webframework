#include <Engine/Core/Log.h>
#include <Engine/Network/Endpoint.h>
#include <Engine/Network/NetSocketBuilder.h>
#include <Engine/Network/NetSocketBase.h>
#include "AddressBinder.h"
#include "ISsl.h"

namespace Neko::Skylar
{
    AddressBinder::AddressBinder(IAllocator& allocator)
        : Ports(allocator)
        , Allocator(allocator)
    {
        this->Ssl = ISsl::Create(Allocator);
    }

    AddressBinder::~AddressBinder()
    {
        Destroy();
    }

    Net::NetSocketBase* AddressBinder::BindPort(const uint16 port)
    {
        if (Ports.Contains(port))
        {
            LogError("Skylar") << "Unable to bind a socket with the used port " << port << ".";

            return nullptr;
        }

        Net::Endpoint address;

        const int32 maxBacklog = SOMAXCONN;

        String temp("127.0.0.1");
        StaticString<48> ResolvedAddressString;
        if (Net::Endpoint localAddress; localAddress.Resolve(*temp, Net::NetworkAddressType::Unspec))
        {
            ResolvedAddressString.Set(localAddress.ToString());
            LogInfo("Skylar") << "Resolved address: " << *ResolvedAddressString;
        }

        address.Parse(*ResolvedAddressString, Net::NetworkAddressType::Ipv4, port);

        // setup socket
        auto* socket = Net::TcpSocketBuilder()
            .Lingering(0)
            .WithBlocking(false)
            .AsReusable(true)
            .BoundTo(address)
            .Listening(maxBacklog)
            .Build(Allocator);

        if (socket == nullptr)
        {
            LogError("Skylar") << "Unable to start at " << ResolvedAddressString << ":" << port << ".";

            return nullptr;
        }

        Ports.Push(port);

        return socket;
    }

    void AddressBinder::Bind(SkylarOptions& options, OnBindCallback onBind)
    {
        auto& listenOptions = options.ListenOptions;

        for (auto& option : listenOptions)
        {
            if (option.IsTls)
            {
                // initialize it first
                auto* context = this->Ssl->InitSslFor(option.SslOptions);
                auto* transport = BindPort(option.Port);
                if (context != nullptr && transport != nullptr)
                {
                    // save context
                    this->Ssl->AddSession(option.Port, context);
                    onBind(option, transport);
                }
            }
            else if (auto* transport = BindPort(option.Port); transport != nullptr)
            {
                onBind(option, transport);
            }
        }
    }

    void AddressBinder::Destroy()
    {
        Clear();
        Ports.Clear();

        if (this->Ssl != nullptr)
        {
            ISsl::Destroy(*this->Ssl);
        }
    }

    void AddressBinder::Clear()
    {
        if (Ssl != nullptr)
        {
            Ssl->Clear();
        }
    }
}
