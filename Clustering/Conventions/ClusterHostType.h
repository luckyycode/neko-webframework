//
//  ClusterHostType.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"
#include "Engine/Utilities/NekoString.h"


namespace Neko::Clustering
{
    enum class ClusterHostType : int8
    {
        /** this is the primary cluster host, controls all secondaries */
        Primary = 0,
        Secondary
    };
}

