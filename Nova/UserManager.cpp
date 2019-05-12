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
//
//  UserManager.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Engine/Core/Log.h"

#include "UserManager.h"
#include "SessionManager.h"
#include "User.h"

#define SESSION_USER_NAME   "loginUserName"

namespace Neko::Nova
{
    UserManager::UserManager(class SessionManager& sessionManager)
    : SessionManager(sessionManager)
    {
    }
    
    bool UserManager::IsUserAuthenticated(Session& session)
    {
        return session.HasKey(SESSION_USER_NAME);
    }
    
    bool UserManager::Login(const IUser& user, Session& session)
    {
        if (user.GetPrimaryKey().IsEmpty())
        {
            LogWarning.log("Nova") << "User identity key is empty!";
            return false;
        }
        
        if (IsUserAuthenticated(session))
        {
            LogInfo.log("Nova") << "User is already authenticated!";
            return true;
        }
        
        session.Insert(SESSION_USER_NAME, user.GetPrimaryKey());
        
        LogInfo.log("Nova") << "User logged in!";
        
        return true;
    }
    
    void UserManager::Logout(Session& session)
    {
        if (const auto keyIt = session.Find(SESSION_USER_NAME); keyIt.IsValid())
        {
            LogInfo.log("Nova") << "User logged out.";
            session.Erase(keyIt);
        }
    }
}

