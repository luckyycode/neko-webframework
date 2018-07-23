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
//  UserManager.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "../../Engine/Core/Log.h"

#include "UserManager.h"
#include "SessionManager.h"
#include "User.h"

#define SESSION_USER_NAME   "loginUserName"

namespace Neko
{
    namespace Nova
    {
        UserManager::UserManager(class SessionManager& sessionManager)
        : SessionManager(sessionManager)
        {
            
        }
        
        bool UserManager::IsUserAuthenticated(Session& session)
        {
            auto keyIt = session.Find(SESSION_USER_NAME);
            return keyIt.IsValid();
        }
        
        bool UserManager::UserLogin(const IUser& user, Session& session)
        {
            if (user.GetIdentityKey().IsEmpty())
            {
                LogWarning.log("Nova") << "User identity key is empty!";
                return false;
            }
            
            if (IsUserAuthenticated(session))
            {
                LogInfo.log("Nova") << "User is already authenticated!";
                return true;
            }
            
            session.Insert(SESSION_USER_NAME, user.GetIdentityKey());
            
            LogInfo.log("Nova") << "User logged in!";
            
            return true;
        }
        
        void UserManager::UserLogout(Session& session)
        {
            if (const auto keyIt = session.Find(SESSION_USER_NAME); keyIt.IsValid())
            {
                LogInfo.log("Nova") << "User logged out.";
                session.Erase(keyIt);
            }
        }
    }
}
