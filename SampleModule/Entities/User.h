#pragma once

#include "../../Nova/User.h"

namespace Neko
{
    class User : public Nova::IUser
    {
    public:
        
        virtual ~User()
        {
            
        }
        
        virtual const String& GetPrimaryKey() const override { return this->UserName; }
        
    public:
        
        String UserName;
        
        String PhoneNumber;
    };
}
