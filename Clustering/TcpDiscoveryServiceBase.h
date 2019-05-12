//
//  TcpDiscoveryServiceBase.h
//  Neko Framework
//
//  Copyright © 2019 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Data/DefaultAllocator.h"
#include "Engine/Network/NetSocketBase.h"
#include "Engine/Containers/DelegateList.h"

#include "IDiscoveryService.h"

namespace Neko::Net
{
    struct LocalAddress;
}

namespace Neko::Mako::Discovery
{
    const static int32 SocketTimeout = 3;
    
    /** Base tcp discovery service. */
    class TcpDiscoveryServiceBase : public IDiscoveryService
    {
    public:
        
        TcpDiscoveryServiceBase()
            : Listeners(GetThreadDefaultAllocator())
            , LocalAddressPtr(nullptr)
            , MaxTimeout(SocketTimeout)
            //, MaxAcknowledgeTimeout(5)
        { }
        
        const Net::NetSocketBase& GetClientSocket() const { return this->MulticastSenderSocket; }
        const Net::Endpoint& GetListenerEndpoint() const { return this->Endpoint; }
        DiscoveryListeners& GetDiscoveryListeners() { return this->Listeners; }
        
    public:
        
        /** Local port to start discovery service. */
        uint16 LocalPort;
        
        /** The local address. If provided address is non-loopback then
            multicast socket is bound to this interface. */
        Net::LocalAddress* LocalAddressPtr;
        
        /** Group socket endpoint. */
        Net::Endpoint Endpoint;
        
        /** Broadcaster socket. */
        Net::NetSocketBase MulticastSocket;
        /** Listener socket. */
        Net::NetSocketBase MulticastSenderSocket;
        
        /** Timeout for receiving acknowledgement for a sent message, in seconds. */
        //uint32 MaxAcknowledgeTimeout;
        
        /** Join timeout, in seconds. */
        int32 MaxTimeout;
        
        DiscoveryListeners Listeners;
    };
}

