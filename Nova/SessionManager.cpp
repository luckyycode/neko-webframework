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
//  SessionManager.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "../../Engine/Core/Log.h"
#include "../../Engine/Utilities/Date.h"

#include "Options.h"
#include "SessionStorageFactory.h"
#include "ISessionStorage.h"
#include "SessionManager.h"
#include "Session.h"

namespace Neko
{
    namespace Nova
    {
        bool SessionManager::Store(Session& session)
        {
            assert(!session.GetId().IsEmpty());
            
            bool result = false;
            ISessionStorage* storage = SessionStorageFactory::Get(GetStoreType());
            
            if (storage != nullptr)
            {
                result = storage->Store(session);
                SessionStorageFactory::Cleanup(GetStoreType(), *storage);
            }
            else
            {
                LogError.log("Nova") << "Session storage " << GetStoreType() << " not found";
            }
            
            return result;
        }
        
        bool SessionManager::Remove(const String& sessionId)
        {
            if (!sessionId.IsEmpty())
            {
                ISessionStorage* storage = SessionStorageFactory::Get(GetStoreType());
                
                if (storage != nullptr)
                {
                    bool result = storage->Remove(sessionId);
                    SessionStorageFactory::Cleanup(GetStoreType(), *storage);
                    
                    return result;
                }
                else
                {
                    LogError.log("Nova") << "Session storage \"" << GetStoreType() << "\" not found";
                }
            }
            
            return false;
        }
        
        Session SessionManager::FindSession(const String& sessionId)
        {
            Session session;
            
            if (!sessionId.IsEmpty())
            {
                ISessionStorage* storage = SessionStorageFactory::Get(GetStoreType());
                
                if (storage != nullptr)
                {
                    session = storage->Find(sessionId);
                    SessionStorageFactory::Cleanup(GetStoreType(), *storage);
                }
                else
                {
                    LogError.log("Nova") << "Session storage \"" << GetStoreType() << "\" not found";
                }
            }
            
            return session;
        }
        
        void SessionManager::ClearSessionsCache()
        {
            static int16 probability = -1;
            
            if (probability == -1)
            {
                probability = Options::SessionOptions().GcProbability;
            }
            
            if (probability > 0)
            {
                int16 result = Math::rand(0, probability - 1);
                
                if (result == 0)
                {
                    LogInfo.log("Nova") << "Clearning session cache...";
                    
                    ISessionStorage* storage = SessionStorageFactory::Get(GetStoreType());
                    if (storage != nullptr)
                    {
                        uint32 cacheLifetime = Options::SessionOptions().MaxGcLifetime;
                        
                        DateTime expiryDate = DateTime::UtcNow();
                        TimeValue lifeTimeValue((int64)cacheLifetime);
                        
                        auto expireDate = expiryDate - lifeTimeValue;
                        
                        storage->ClearCache(expireDate);
                        
                        SessionStorageFactory::Cleanup(GetStoreType(), *storage);
                    }
                }
            }
        }
        
        void SessionManager::SetCsrfProtectionData(Session& session)
        {
            const String& type = Options::SessionOptions().StorageType;
            
            if (type == "cookie")
            {
                const String& key = Options::SessionOptions().CsrfKey;
                session.Insert(key, GenerateSessionId());
            }
        }
        
        void SessionManager::ResetSession(Session& session)
        {
            session.Reset();
            
            SetCsrfProtectionData(session);
        }
        
        String CreateHash()
        {
            // @todo more specific
            uint64 hash = Math::RandGUID();
            String result;
            result += hash;
            return result;
        }
        
        String SessionManager::GenerateSessionId()
        {
            String sessionId;
            
            int16 i;
            
            for (i = 0; i < 3; ++i)
            {
                sessionId = CreateHash();
                if (FindSession(sessionId).IsEmpty())
                {
                    break;
                }
            }
            
            if (i == 3)
            {
                LogError.log("Nova") << "Couldn't generate session UID!";
            }
            
            return sessionId;
        }
        
        const String& SessionManager::GetStoreType() const
        {
            const auto& type = Options::SessionOptions().StorageType;
            return type;
        }
    }
}
