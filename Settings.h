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
//  Settings.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "../Engine/Core/Module.h"
#include "ContentTypes/IContentType.h"

#include "ApplicationSettings.h"

namespace Neko
{
    namespace FS
    {
        class FileSystem;
    }
    
    namespace Http
    {
        /// Common server settings.
        class ServerSettings
        {
            /** Server application list. */
            class ApplicationsList
            {
            public:
                
                ApplicationsList(IAllocator& allocator);
                
                /** Adds application in the list */
                void AddApplication(const String& name, ApplicationSettings* settings);
                
                /** Recursively looks up for appsettings. */
                ApplicationSettings* Find(const String& name) const;
                
                /** Collects all settings from every node. */
                void GetAllApplicationSettings(TArray< ApplicationSettings* >& applications) const;
                
                void Clear();
                
                NEKO_FORCE_INLINE bool IsEmpty() const { return List.IsEmpty(); }
                
            public:
                
                struct ApplicationSettings* ApplicationSettings;
                
                THashMap< String, ApplicationsList* > List;
                
                IAllocator& Allocator;
            };
            
        public:
            
            ServerSettings(FS::FileSystem& fileSystem, IAllocator& allocator);
            
            ~ServerSettings();
            
            /** Adds content type to supported content types list. */
            void AddContentType(IContentType* contentType);
            
            void Clear();
            
        private:
            
            /** Adds application which will be used by server to lookup. */
            bool AddApplication(const String& application, ApplicationSettings* settings);
            
        public:
            
            /** Returns all applications. */
            void GetAllApplicationSettings(TArray< ServerSettings::ApplicationsList* >& applications);
            
            /**
             * Configuration parsing for each app.
             */
            bool LoadAppSettings(const String& file, TArray<Module>& modules);
            
            bool SetApplicationModuleMethods(ApplicationSettings& settings, Module& module);
            
            int32 LoadModule(const String& name, const String& rootDirectory, TArray<Module>& modules, ApplicationSettings& settings);
            
        public:
            
            //! Maximum amount of threads that server can use. If 0, we'll machine available cores.
            int32 ThreadsMaxCount;
        
            /** Supported content variants. */
            THashMap< String, IContentType* > ContentTypes;
            
            /** Supported mime types. */
            THashMap< String, String > SupportedMimeTypes;
            
            //! Application list
            ApplicationsList List;
            
        private:
            
            IAllocator& Allocator;
            FS::FileSystem& FileSystem;
        };
    }
}

