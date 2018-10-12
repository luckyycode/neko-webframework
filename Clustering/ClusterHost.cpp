//
//  ClusterHost.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "ClusterHost.h"
#include "Engine/Core/Log.h"

namespace Neko::Clustering
{
    ClusterHost::ClusterHost(const String& name, ClusterHostType type)
    : Name(name)
    , Type(type)
    , IsStarted(false)
    , DeploymentId(0)
    {
        Type = NodeConfiguration.IsPrimary
            ? ClusterHostType::Primary : ClusterHostType::Secondary;
    }
    
    bool ClusterHost::Start()
    {
        if (IsStarted)
        {
            LogWarning.log("Cluster") << "Current cluster host had already started.";
            return false;
        }
        
        auto& endPoint = NodeConfiguration.EndPoint;
        
        if (endPoint.AddressType == ENetworkAddressType::NA_BAD)
        {
            LogWarning.log("Cluster") << "Attempt to start cluster host with incorrect endpoint.";
            return false;
        }
        
        Socket.Init(endPoint, ESocketType::TCP);
        if (Socket.GetNativeHandle() == INVALID_SOCKET)
        {
            LogError.log("Cluster") << "Cluster host couldn't start at " << endPoint.ToString();
            return false;
        }
        
        return true;
    }
    
    void ClusterHost::Stop() { }
}

