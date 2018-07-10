//
//          *                  *
//             __                *
//           ,db'    *     *
//          ,d8/       *        *    *
//          888
//          `db\       *     *
//            `o`_                    **
//               *                 / )
//             *    /\__/\ *       ( (  *
//           ,-.,-.,)    (.,-.,-.,-.) ).,-.,-.
//          | @|  ={      }= | @|  / / | @|o |
//         _j__j__j_)     `-------/ /__j__j__j_
//          ________(               /___________
//          |  | @| \              || o|O | @|
//          |o |  |,'\       ,   ,'"|  |  |  |  hjw
//          vV\|/vV|`-'\  ,---\   | \Vv\hjwVv\//v
//                     _) )    `. \ /
//                    (__/       ) )
//  _   _      _           _____                                            _
// | \ | | ___| | _____   |  ___| __ __ _ _ __ ___   _____      _____  _ __| | __
// |  \| |/ _ \ |/ / _ \  | |_ | '__/ _` | '_ ` _ \ / _ \ \ /\ / / _ \| '__| |/ /
// | |\  |  __/   < (_) | |  _|| | | (_| | | | | | |  __/\ V  V / (_) | |  |   <
// |_| \_|\___|_|\_\___/  |_|  |_|  \__,_|_| |_| |_|\___| \_/\_/ \___/|_|  |_|\_\
//
//  Model.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../../Engine/Utilities/NekoString.h"
#include "../../Engine/RTTI/IReflectable.h"
#include "../../Engine/RTTI/IReflectionType.h"

namespace Neko
{
    namespace Mvc
    {
        struct IModel : IReflectable
        {
        public:
            DECLARE_RTTI_BASE(IModel)
        };
        
        class IModelReflection : public IReflectionType<IModel, IReflectable, IModelReflection>
        {
        private:
            
        public:
            
            IModelReflection()
            {
            }
            
            const String& GetName() override
            {
                static String Name = "IModel";
                return Name;
            }
            
            uint32 GetId() override
            {
                return 13337;
            }
            
            TSharedPtr<IReflectable> NewObject() override
            {
                assert(false && "Can't create abstract model");
                return nullptr;
            }
        };
    }
}
