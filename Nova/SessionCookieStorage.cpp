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
//  SessionCookieStorage.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "SessionCookieStorage.h"

#include "Engine/Core/Log.h"
#include "Engine/Data/Blob.h"
#include "Engine/Utilities/Utilities.h"

namespace Neko
{
    namespace Nova
    {
        bool SessionCookieStorage::Store(Session& session)
        {
            if (session.IsEmpty())
            {
                return true;
            }
            
            const uint32 size = InputBlob::GetContainerSize(session);
            
            String sessionData(size);
            
            // Write session data
            OutputBlob data((void* )*sessionData, INT_MAX);
            data << *static_cast<const SessionMap* >(&session);
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
            Session session(sessionId);
            if (sessionId.IsEmpty())
            {
                return session;
            }
            
            TArray<String> rawData;
            sessionId.ParseIntoArray(rawData, "_", false);
            
            if (rawData.GetSize() == 2)
            {
                String unhexSessionData(rawData[0].Length());
                
                Util::HexToBytes(rawData[0], (uint8* )*unhexSessionData);
                
                String sessionData;
                sessionData.Append(*unhexSessionData, unhexSessionData.GetCapacity() / 2);
                sessionData += Options::SessionOptions().Secret;
                
                const uint32 dataHash = Crc32(*sessionData, sessionData.Length());
                const uint32 digest = StringToUnsignedLong(*rawData[1]);
                
                if (dataHash != digest)
                {
//                    GLogWarning.log("Nova") << "Probably a tampered cookie detected!";
                    return session;
                }
                
                InputBlob data((void* )*unhexSessionData, INT_MAX);
                data >> *static_cast<SessionMap* >(&session);
            }
            return session;
        }
    }
}
