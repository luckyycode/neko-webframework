#pragma once

namespace Neko
{
    namespace Mvc
    {
        class SessionManager;
        class Session;
        class IUser;
        
        /// Provides methods for managing users and user sessions.
        class UserManager
        {
        public:
            
            UserManager(SessionManager& sessionManager);
            
            /**
             * Checks if session contains authorized user data.
             *
             * @param session   User session.
             * @return  TRUE if user is logged in, false otherwise.
             */
            bool IsUserAuthenticated(Session& session);
            
            /**
             * Logs user in.
             *
             * @param user      A user to log in.
             * @param session   User session.
             * @return  TRUE if user had been successfully logged or if already logged in, false otherwise.
             */
            bool UserLogin(const IUser& user, Session& session);
            
            /**
             * Logs user session out.
             */
            void UserLogout(Session& session);
            
        private:
            
            SessionManager& SessionManager;
        };
    }
}
