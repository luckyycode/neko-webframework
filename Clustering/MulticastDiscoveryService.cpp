//
//  MulticastDiscoveryService.cpp
//  Neko Framework
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#include "Engine/Core/Log.h"
#include "Engine/Data/Blob.h"
#include "Engine/Network/LocalAddress.h"
#include "Engine/Network/Network.h"
#include "Engine/Utilities/NekoString.h"
#include "MulticastDiscoveryService.h"

namespace Neko::Mako::Discovery
{
    const static char* DefaultMulticastGroup = "224.2.2.4";
    const static uint16 DefaultMulticastPort = 51700;
    
    MulticastDiscoveryService::MulticastDiscoveryService()
        : TcpDiscoveryServiceBase()
        , MulticastGroup(DefaultMulticastGroup)
        , TimeToLive(255)
    {
        LocalPort = DefaultMulticastPort;

    }
    
    MulticastDiscoveryService::~MulticastDiscoveryService()
    {
    }
    
    bool MulticastDiscoveryService::Configure()
    {
        LocalAddressPtr = Net::GetLocalAddress(nullptr, Net::NetworkAddressType::Ipv4);
        auto* localAddress = *LocalAddressPtr->ToString();
        
        // setup multicast
        auto endpoint = MulticastSocket.Init(nullptr, LocalPort, Net::SocketType::Udp);
        
        MulticastSocket.MakeNonBlocking(true);
        MulticastSocket.SetReuseAddress();
        MulticastSocket.SetReusePort();
        MulticastSocket.SetMulticastTtl(TimeToLive);
        
        bool result = MulticastSocket.Bind(endpoint);
        
        // join the group
        Net::Endpoint groupEndpoint(*MulticastGroup, Net::NetworkAddressType::Multicast);
        result = MulticastSocket.JoinMulticastGroup(groupEndpoint); // @todo locate first
        
        LogInfo.log("Mako") << localAddress << " successfully joined multicast group "
            << *MulticastGroup << " on port " << LocalPort;
        
        // setup consumer (listener)
        endpoint = MulticastSenderSocket.Init(nullptr, LocalPort, Net::SocketType::Udp);
        MulticastSenderSocket.MakeNonBlocking(true);
        MulticastSenderSocket.SetReuseAddress();
        MulticastSenderSocket.SetReusePort();
        
        result = MulticastSenderSocket.Bind(endpoint);
        
        Endpoint.Resolve(*MulticastGroup, Net::NetworkAddressType::Ipv4);
        Endpoint.Port = LocalPort;
        
        if (result == false)
        {
            LogError.log("Mako") << "Couldn't configure multicast discovery";
            return false;
        }
        
        LogInfo.log("Mako") << "Configured multicast discovery.";
        return true;
    }
    
    void MulticastDiscoveryService::Broadcast(const OutputData& data, const Net::Endpoint* endpoint)
    {
        if (endpoint)
        {
            printf("broadcast to %s:%d\n", *endpoint->ToString(), endpoint->Port);
        }
        bool succeeded = MulticastSenderSocket.SendPacket(endpoint != nullptr ? endpoint : &Endpoint, data.GetData(), data.GetPos());
        NEKO_UNUSED(succeeded);
    }
    
    void MulticastDiscoveryService::ListenBroadcast()
    {
        Net::Endpoint endpoint;
        
        long size = 0;
        char data[512];
        
        bool succeeded = MulticastSocket.GetPacketBlocking(&endpoint, data, size, sizeof(data), MaxTimeout);
        
        if (succeeded)
        {
            InputData inputData(data, size);
            Listeners.Invoke(inputData, endpoint);
        }
    }
}


