//
//  NodeConfiguration.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"
#include "Engine/Utilities/NekoString.h"
#include "Engine/Network/NetSocket.h"

namespace Neko::Clustering
{
    using namespace Neko::Net;
    
    class NodeConfiguration
    {
    public:
        
        NodeConfiguration();
        
        void ConfigureEndPoint(const INetSocket& socket);
        
    public:
        
        bool IsPrimary;
        
        NetAddress EndPoint;
    };
}

