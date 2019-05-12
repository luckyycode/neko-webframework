//
//  Mako.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"
#include "Engine/Utilities/Utilities.h"

namespace Neko::Mako
{
    class IMako
    {
    public:
        virtual ~IMako() { }
        
        virtual int64 GetId() const = 0;
        
        static IMako* Create();
    };
}

