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
//  Session.h
//  Neko SDK
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Containers/HashMap.h"
#include "Engine/Data/Blob.h"
#include "Options.h"

namespace Neko::Nova
{
    /** Http session. */
    class Session
    {
        using SessionMap = THashMap<String, String>;
        using KeyIterator = SessionMap::iterator;
        
    public:
        inline Session(IAllocator& allocator, const String& sessionId = String::Empty)
        : Id(sessionId)
        , Storage(allocator)
        { }
        
        inline Session(const Session& other)
        : Storage(other.Storage)
        , Id(other.Id)
        { }
        
        Session& operator = (const Session& other);
        
        /** Returns this session id. */
        NEKO_FORCE_INLINE String GetId() const { return Id; }
        
        /** Insers new key-value pair. */
        void Insert(const String& key, const String& value);
        
        const String GetValue(const String& key);
        const String GetValue(const String& key, const String& defaultValue);
        
        /** Shared session name. */
        static String GetSessionName();
        
        /** Returns TRUE if session id is not empty. */
        NEKO_FORCE_INLINE bool IsValid() const { return !this->Id.IsEmpty(); }
        NEKO_FORCE_INLINE bool IsEmpty() const { return Storage.IsEmpty(); }
        
        NEKO_FORCE_INLINE const SessionMap& GetKeyValues() const { return Storage; }
        
        bool HasKey(const String& key);
        
        KeyIterator Find(const String& key);
        void Erase(KeyIterator iterator);
        
        inline void operator << (OutputData& data)
        {
            data << Storage;
        }
        
        inline void operator >> (InputData& data)
        {
            data >> Storage;
        }
        
    private:
        friend class SessionManager;
        
        // Key-Value Data.
        SessionMap Storage;
        
        /** Resets this session. */
        void Reset();
        
    public:
        String Id;
    };
    
    inline Session& Session::operator = (const Session& other)
    {
        Storage = other.Storage;
        Id = other.Id;
        
        return *this;
    }
    
    inline void Session::Insert(const String& key, const String& value)
    {
        auto it = Storage.Find(key);
        if (not it.IsValid())
        {
            Storage.Insert(key, value);
        }
        else
        {
            it.value() = value;
        }
    }
    
    inline const String Session::GetValue(const String& key)
    {
        return Storage.at(key);
    }
    
    inline const String Session::GetValue(const String& key, const String& defaultValue)
    {
        auto it = Storage.Find(key);
        return not it.IsValid() ? defaultValue : it.value();
    }
    
    inline String Session::GetSessionName()
    {
        return Options::SessionOptions().Name;
    }
    
    inline bool Session::HasKey(const String& key)
    {
        auto keyIt = Find(key);
        return keyIt.IsValid();
    }
    
    inline Session::KeyIterator Session::Find(const String& key)
    {
        return Storage.Find(key);
    }
    
    inline void Session::Erase(Session::KeyIterator iterator)
    {
        Storage.Erase(iterator);
    }
}

