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
//  SessionManager.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Engine/Core/Log.h"
#include "Engine/Utilities/Date.h"

#include "Options.h"
#include "SessionStorageFactory.h"
#include "ISessionStorage.h"
#include "SessionManager.h"
#include "Session.h"

namespace Neko::Nova
{
    bool SessionManager::Store(Session& session)
    {
        assert(not session.GetId().IsEmpty());
        
        bool result = false;
        auto* storage = SessionStorageFactory::Get(GetStoreType(), Allocator);
        
        if (storage != nullptr)
        {
            result = storage->Store(session);
            SessionStorageFactory::Cleanup(GetStoreType(), *storage);
        }
        else
        {
            LogError.log("Nova") << "Session storage " << (int)GetStoreType() << " not found";
        }
        
        return result;
    }
    
    bool SessionManager::Remove(const String& sessionId)
    {
        if (not sessionId.IsEmpty())
        {
            auto* storage = SessionStorageFactory::Get(GetStoreType(), Allocator);
            
            if (storage != nullptr)
            {
                bool result = storage->Remove(sessionId);
                SessionStorageFactory::Cleanup(GetStoreType(), *storage);
                
                return result;
            }
            else
            {
                LogError.log("Nova") << "Session storage \"" << (int)GetStoreType() << "\" not found";
            }
        }
        
        return false;
    }
    
    
    Session SessionManager::FindSession(const String& sessionId)
    {
        Session session(Allocator);
        
        if (not sessionId.IsEmpty())
        {
            auto* storage = SessionStorageFactory::Get(GetStoreType(), Allocator);
            
            if (storage != nullptr)
            {
                session = storage->Find(sessionId);
                SessionStorageFactory::Cleanup(GetStoreType(), *storage);
            }
            else
            {
                LogError.log("Nova") << "Session storage \"" << (int)GetStoreType() << "\" not found";
            }
        }
        
        return session;
    }
    
    void SessionManager::ClearSessionsCache()
    {
        static int16 probability = 5;
        
        int32 result = Math::rand(0, probability - 1);
        
        if (result == 0)
        {
            LogInfo.log("Nova") << "Clearing session cache...";
            
            auto* storage = SessionStorageFactory::Get(GetStoreType(), Allocator);
            if (storage != nullptr)
            {
                uint32 cacheLifetime = 1000;
                
                auto expiryDate = DateTime::UtcNow();
                TimeValue lifeTimeValue((int64)cacheLifetime);
                
                auto expireDate = expiryDate - lifeTimeValue;
                
                storage->ClearCache(expireDate);
                
                SessionStorageFactory::Cleanup(GetStoreType(), *storage);
            }
        }
    }
    
    void SessionManager::SetCsrfProtectionData(Session& session)
    {
        const auto& type = Options::SessionOptions().StorageType;
        
        if (type == SessionStorageType::Cookie)
        {
            const auto& key = Options::SessionOptions().CsrfKey;
            session.Insert(key, NewSessionId());
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
        uint64 hash = Math::RandGuid();
        String result;
        result += hash;
        return result;
    }
    
    String SessionManager::NewSessionId()
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
    
    const SessionStorageType SessionManager::GetStoreType() const
    {
        return Options::SessionOptions().StorageType;
    }
}

