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
//  Session.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../../Engine/Containers/HashMap.h"

#include "Options.h"

namespace Neko
{
    namespace Nova
    {
        typedef THashMap<String, String> SessionMap;
        
        /// Http session. Key-value data.
        class Session : public SessionMap
        {
        public:
            
            Session(const String& sessionId = String::Empty);
            Session(const Session& other);
            
            Session& operator = (const Session& other);
            
            /** Returns this session id. */
            NEKO_FORCE_INLINE String GetId() const { return Id; }
            
            /** Insers new key-value pair. */
            iterator Insert(const String& key, const String& value);
            
            const String GetValue(const String& key);
            const String GetValue(const String& key, const String& defaultValue);
            
            /** Shared session name. */
            static String GetSessionName();

            /** Returns TRUE if session id is not empty. */
            NEKO_FORCE_INLINE bool IsValid() const { return !this->Id.IsEmpty(); }
            
        private:
            
            friend class SessionManager;
            
            /** Resets this session. */
            void Reset();
            
        public:
            
            String Id;
        };
        
        inline Session::Session(const String& sessionId)
        : Id(sessionId)
        {
            
        }
        
        inline Session::Session(const Session& other)
        : SessionMap(*static_cast<const SessionMap* >(&other))
        , Id(other.Id)
        {
            
        }
        
        inline Session& Session::operator = (const Session& other)
        {
            SessionMap::operator = (*static_cast<const SessionMap* >(&other));
            Id = other.Id;
            
            return *this;
        }
        
        inline Session::iterator Session::Insert(const String& key, const String& value)
        {
            auto it = SessionMap::Find(key);
            if (!it.IsValid())
            {
                return SessionMap::InsertEx(key, value);
            }
            else
            {
                it.value() = value;
                return it;
            }
        }
        
        inline const String Session::GetValue(const String& key)
        {
            return SessionMap::at(key);
        }
        
        inline const String Session::GetValue(const String& key, const String& defaultValue)
        {
            auto it = SessionMap::Find(key);
            if (!it.IsValid())
            {
                return defaultValue;
            }
            return it.value();
        }
        
        inline String Session::GetSessionName()
        {
            return Options::SessionOptions().Name;
        }
    }
}
