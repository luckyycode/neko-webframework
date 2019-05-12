//
//  Cluster.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Mako.h"

#include "Engine/Network/NetSocketV6.h"
#include "Engine/Utilities/Templates.h"
#include <set>
#include <ostream>

#include "../kek/common.h"
#include "MakoConfiguration.h"
#include "ClusterHost.h"

#define NET_MULTICAST_IP6 "ff04::696f:7175:616b:6533"

namespace Neko::Mako
{
    class Mako : public IMako
    {
    public:
        Mako()
        {
            MakoConfiguration config;
            config.SetDiscoverer(nullptr);
            
            Start(config);
        }
        
        void Start(MakoConfiguration configuration)
        {
            Configuration = Neko::Move(configuration);
            
            ClusterHostPtr = new ClusterHost("MachineNode", *configuration.DiscovererPtr);
            ClusterHostPtr->Start();
        }
        
    public:
        ClusterHost* ClusterHostPtr;
        
        MakoConfiguration Configuration;
        
        int64 GetId() const
        {
            return 0;
        }
    };
    
    IMako* IMako::Create()
    {
        return new Mako();
    }
}

