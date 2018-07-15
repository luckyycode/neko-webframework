#pragma once

#include "../../Mvc/User.h"

namespace Neko
{
    class User : public Mvc::IUser
    {
    public:
        
        virtual ~User()
        {
            
        }
        
        virtual const String& GetIdentityKey() const override { return this->UserName; }
        
    public:
        
        String UserName;
        
        String PhoneNumber;
    };
}
