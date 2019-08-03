#pragma once

#include <Engine/Core/Module.h>
#include "IServer.h"
#include "../PoolApplicationSettings.h"

namespace Neko::Skylar
{
    class Server;

    /** Module manager. Used for in-process hosting. */
    class ModuleManager
    {
    public:
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
            THashMap< uint32, ApplicationsList* > List;
            IAllocator& Allocator;
        };

        ModuleManager(Server& server, IAllocator& allocator, FileSystem::IFileSystem& fileSystem);
        ~ModuleManager();

        bool LoadSettings();

        /** Prepares applications. Binds found application ports. */
        void PrepareApplications();

        /** Updates all server modules. */
        void UpdateApplications();

        /** Updates the specified server module. */
        bool UpdateApplication(Module& module, TArray< PoolApplicationSettings* >& applications, const uint32 moduleIndex);

        void Cleanup();

    private:
        /** Adds application which will be used by server to lookup. */
        bool AddApplication(const String& application, PoolApplicationSettings* settings);
    public:
        bool SetApplicationModuleMethods(PoolApplicationSettings& settings, Module& module);

        int16 LoadModule(const String& name, const char* rootDirectory, TArray< Module >& modules,
                PoolApplicationSettings& settings);
        void GetAllApplicationSettings(TArray< PoolApplicationSettings* >& applications);

    private:
        /** Configuration parsing for each app. */
        bool LoadAppSettings(const String& file, TArray< Module >& modules);

    private:
        /** Loaded server modules. */
        TArray< Module > Modules;

        IAllocator& Allocator;
        FileSystem::IFileSystem& FileSystem;

        Server& Server;

    public:
        /** Application list */
        ApplicationsList List;
    };
}
