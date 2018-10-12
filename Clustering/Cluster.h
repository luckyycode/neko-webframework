//
//  Cluster.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"
#include "Engine/Utilities/NekoString.h"

namespace Neko::Clustering
{
    class ICluster
    {
    public:
        
        virtual ~ICluster() { }
        
        virtual int64 GetId() const = 0;
    };
}

