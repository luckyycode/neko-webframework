
#include "../../Engine/Core/Log.h"

#include "UserManager.h"
#include "SessionManager.h"
#include "User.h"

#define SESSION_USER_NAME   "loginUserName"

namespace Neko
{
    namespace Mvc
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
                GLogWarning.log("Mvc") << "User identity key is empty!";
                return false;
            }
            
            if (IsUserAuthenticated(session))
            {
                GLogInfo.log("Mvc") << "User is already authenticated!";
                return true;
            }
            
            session.Insert(SESSION_USER_NAME, user.GetIdentityKey());
            
            GLogInfo.log("Mvc") << "User logged in!";
            
            return true;
        }
        
        void UserManager::UserLogout(Session& session)
        {
            const auto keyIt = session.Find(SESSION_USER_NAME);
            if (keyIt.IsValid())
            {
                GLogInfo.log("Mvc") << "User logged out.";
                session.Erase(keyIt);
            }
        }
    }
}
