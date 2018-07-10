#pragma once

#include "../../Mvc/Model.h"

namespace Neko
{
    struct ChatModel
    {
        StaticString<64> LastName;
        long Id;
        StaticString<64> FirstName;
        StaticString<32> UserName;
    };
    
    struct MessageModel
    {
        uint64 Date;
        ChatModel Chat;
        
        long MessageId;
        
        ChatModel From;
        String Text;
    };
    
    struct UpdateModel : Mvc::IModel
    {
        long UpdateId;
        MessageModel Message;
    };
}
