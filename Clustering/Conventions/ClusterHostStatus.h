//
//  ClusterHostType.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"
#include "Engine/Utilities/NekoString.h"


namespace Neko::Mako
{
    enum class ClusterHostStatus : int8
    {
        None = 0,
        
        Created = 1,
        
        Joining = 2,
        
        Active = 3,
        
        ShuttingDown,
        
        Stopped
    };
}

