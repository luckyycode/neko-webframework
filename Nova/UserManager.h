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
//  UserManager.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

namespace Neko::Nova
{
    class SessionManager;
    class Session;
    class IUser;
    
    /** Provides methods for managing users and user sessions. */
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
        bool Login(const IUser& user, Session& session);
        
        /**
         * Logs user session out.
         */
        void Logout(Session& session);
        
    private:
        SessionManager& SessionManager;
    };
}

