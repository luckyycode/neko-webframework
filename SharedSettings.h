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
//  SharedSettings.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Engine/Core/Module.h"

#include "ContentTypes/IContentType.h"
#include "PoolApplicationSettings.h"

namespace Neko::FileSystem { class IFileSystem; }
namespace Neko::Skylar
{
    /** Common server settings. */
    class ServerSharedSettings
    {
        /** Server application list. */
        struct ApplicationsList
        {
        public:
            ApplicationsList(IAllocator& allocator);
            
            /** Adds application in the list */
            void AddApplication(const String& name, PoolApplicationSettings* settings);
            /** Recursively looks up for appsettings. */
            PoolApplicationSettings* Find(const String& name) const;
            /** Collects all settings from every node. */
            void GetAllApplicationSettings(TArray< PoolApplicationSettings* >& applications) const;
            
            void Clear();
            
            NEKO_FORCE_INLINE bool IsEmpty() const { return List.IsEmpty(); }
            
        public:
            struct PoolApplicationSettings* PoolApplicationSettings;
            THashMap< String, ApplicationsList* > List;
            IAllocator& Allocator;
        };
        
    public:
        ServerSharedSettings(FileSystem::IFileSystem& fileSystem, IAllocator& allocator);
        
        ~ServerSharedSettings();
        
        /** Adds content type to supported content types list. */
        void AddContentType(IContentType& contentType);
        void Clear();
        
    private:
        /** Adds application which will be used by server to lookup. */
        bool AddApplication(const String& application, PoolApplicationSettings* settings);
        
    public:
        /** Configuration parsing for each app. */
        bool LoadAppSettings(const String& file, TArray< Module >& modules);
        bool SetApplicationModuleMethods(PoolApplicationSettings& settings, Module& module);
        
        int16 LoadModule(const String& name, const char* rootDirectory, TArray< Module >& modules,
             PoolApplicationSettings& settings);
        void GetAllApplicationSettings(TArray< PoolApplicationSettings* >& applications);
        
    public:
        // @todo extend this
        /** Supported mime types. */
        THashMap< String, String > SupportedMimeTypes;
        /** Supported content variants. */
        THashMap< String, IContentType* > ContentTypes;
        /** Application list */
        ApplicationsList List;
        
        StaticString<Net::MaxAddressStringLength> ResolvedAddressString;
        
        //! Memory size in bytes.
        uint64 MaxMemoryUsage;

    private:
        IAllocator& Allocator;
        FileSystem::IFileSystem& FileSystem;
    };
}


