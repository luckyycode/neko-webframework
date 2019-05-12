//
//  MulticastDiscoveryService.h
//  Neko Framework
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Utilities/NekoString.h"
#include "TcpDiscoveryServiceBase.h"

namespace Neko::Mako::Discovery
{
    /** Multicast cluster discovery service. */
    class MulticastDiscoveryService : public TcpDiscoveryServiceBase
    {
    public:
        
        MulticastDiscoveryService();
        virtual ~MulticastDiscoveryService();
        
        bool Configure() override;
        
        void Broadcast(const OutputData& data, const Net::Endpoint* endpoint = nullptr) override;
        void ListenBroadcast() override;
        
    public:
        
        /** Multicast packet ttl to control the scope of multicast. */
        uint8 TimeToLive;
        
        /** Multicast group address. */
        String MulticastGroup;
    };
}

