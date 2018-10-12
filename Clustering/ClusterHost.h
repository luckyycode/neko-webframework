//
//  Cluster.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"
#include "Engine/Utilities/NekoString.h"
#include "Engine/Network/NetSocket.h"
#include "Conventions/ClusterHostType.h"
#include "NodeConfiguration.h"

namespace Neko::Clustering
{
    using namespace Net;
    
    class ClusterHost
    {
    public:
        
        ClusterHost(const String& name, ClusterHostType type);
        
        virtual ~ClusterHost() { }
        
        bool Start();
        
        void Stop();
        
    public:
        
        bool IsStarted;
        
        //! Cluster id
        int64 DeploymentId;
        
        //! Name of this host
        String Name;
        
        //! Host type
        ClusterHostType Type;
        
        //! Mostly contains network settings for this host
        NodeConfiguration NodeConfiguration;
        
        INetSocket Socket;
    };
}

