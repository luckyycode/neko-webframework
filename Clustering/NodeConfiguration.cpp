//
//  NodeConfiguration.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "NodeConfiguration.h"
#include "Engine/Core/Log.h"

namespace Neko::Clustering
{
    using namespace Neko::Net;
    
    NodeConfiguration::NodeConfiguration()
    : IsPrimary(false)
    {
        
    }
    
    void NodeConfiguration::ConfigureEndPoint(const INetSocket& socket)
    {
        if (socket.GetAddress(EndPoint))
        {
            LogWarning.log("Cluster") << "Resolved cluster host address: " << *EndPoint.ToString();
        }
    }
}

