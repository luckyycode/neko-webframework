//
//  ClusterNodeType.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"
#include "Engine/Utilities/NekoString.h"

namespace Neko::Mako
{
    /** Possible cluster node types. */
    enum struct ClusterNodeType : uint8
    {
        /** Primary zone node. Controls all the secondaries. */
        Primary = 0,
        
        Secondary
    };
}

