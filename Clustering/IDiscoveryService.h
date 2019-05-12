//
//  IDiscoveryService.h
//  Neko Framework
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/DelegateList.h"

namespace Neko
{
    class InputData;
    class OutputData;
    
    namespace Net
    {
        struct Endpoint;
    }
}

namespace Neko::Mako::Discovery
{
    /** Node discovery event type. */
    enum struct DiscoveryEventType
    {
        /** Node has joined the cluster. */
        Join,
        
        /** Node has left the cluster. */
        Leave
    };
    
    struct NodeEvent
    {
        long NodeId;
    };
    
    struct DiscoveryEvent : NodeEvent
    {
        DiscoveryEvent(long nodeId, DiscoveryEventType type) : Type(type)
        {
            NodeId = nodeId;
        }
        
        DiscoveryEventType Type;
    };
    
    using DiscoveryListeners = DelegateList<void (InputData& data, Net::Endpoint& endpoint)>;
    
    /** Cluster membership discovery service. */
    class IDiscoveryService
    {
    public:
        
        virtual bool Configure() = 0;
        
        virtual void Broadcast(const OutputData& data, const Net::Endpoint* endpoint = nullptr) = 0;
        
        virtual void ListenBroadcast() = 0;
        
        virtual const Net::NetSocketBase& GetClientSocket() const = 0;
        virtual const Net::Endpoint& GetListenerEndpoint() const = 0;
        virtual DiscoveryListeners& GetDiscoveryListeners() = 0;
    };
}

