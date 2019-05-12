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
//  SessionCookieStorage.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "SessionCookieStorage.h"

#include "Engine/Core/Log.h"
#include "Engine/Data/Blob.h"
#include "Engine/Utilities/Utilities.h"

namespace Neko::Nova
{
    SessionCookieStorage::SessionCookieStorage(IAllocator& allocator)
        : Allocator(allocator )
    {
    }

    bool SessionCookieStorage::Store(Session& session)
    {
        if (session.IsEmpty())
        {
            return true;
        }
        
        const uint32 size = InputData::GetContainerSize(session.GetKeyValues());
        
        String sessionData(size, Allocator);
        
        // Write session data
        OutputData data((void* )*sessionData, INT_MAX);
        session << data;
        
        // to hex
        session.Id = Util::BytesToHex((uint8* )*sessionData, size);
        
        String digest;
        digest.Append(*sessionData, sessionData.GetCapacity());
        digest += Options::SessionOptions().Secret;
        
        auto hash = Crc32(*digest, digest.Length()); // @todo use sha, this is for temporary purposes
        digest.Clear();
        digest += hash;
        
        auto& sessionId = session.Id;
        sessionId += "_";
        sessionId += digest;
        
        return true;
    }
    
    Session SessionCookieStorage::Find(const String& sessionId)
    {
        Session session(Allocator, sessionId);
        if (sessionId.IsEmpty())
        {
            LogWarning.log("Nova") << "SessionCookieStorage: Empty session id.";
            return session;
        }
        
        TArray<String> rawData(Allocator);
        sessionId.ParseIntoArray(rawData, "_", false);
        
        if (rawData.GetSize() == 2)
        {
            String sessionDataRaw(rawData[0].Length(), Allocator);
            
            Util::HexToBytes(rawData[0], (uint8* )*sessionDataRaw);
            
            String sessionData;
            sessionData.Append(*sessionDataRaw, sessionDataRaw.GetCapacity() / 2);
            sessionData += Options::SessionOptions().Secret;
            
            // @todo sha!
            const uint32 dataHash = Crc32(*sessionData, sessionData.Length());
            const uint32 digest = StringToUnsignedLong(*rawData[1]);
            
            if (dataHash != digest)
            {
                //                    LogWarning.log("Nova") << "Probably a strange cookie detected!";
                return session;
            }
            
            InputData data((void* )*sessionDataRaw, INT_MAX);
            session >> data;
        }
        return session;
    }
}

