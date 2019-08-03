#include <Engine/Core/Log.h>
#include <Engine/Core/Profiler.h>
#include <Engine/Utilities/NekoString.h>
#include <Engine/FileSystem/PlatformFile.h>
#include <Engine/Platform/File.h>
#include <Engine/Data/JsonSerializer.h>
#include <Engine/Platform/Platform.h>
#include "ModuleManager.h"
#include "Server.h"

namespace Neko::Skylar
{
    using namespace Neko::Net;
    static const char* ServerSettingsFileName = "serversettings.json";

    ModuleManager::ModuleManager(class Server& server, IAllocator& allocator, FileSystem::IFileSystem& fileSystem)
        : Modules(allocator)
        , FileSystem(fileSystem)
        , List(allocator)
        , Allocator(allocator)
        , Server(server)
    {
    }

    ModuleManager::~ModuleManager()
    {
        Cleanup();
    }

    bool ModuleManager::LoadSettings()
    {
        // load every application & settings
        if (not LoadAppSettings(ServerSettingsFileName, Modules))
        {
            return false;
        }

        return true;
    }

    void ModuleManager::PrepareApplications()
    {
        LogInfo("Skylar") << "Preparing server applications..";
        // applications settings list
        TArray< PoolApplicationSettings* > applications(Allocator);
        // get full applications settings list
        GetAllApplicationSettings(applications);

        // open applications sockets
        for (auto& application : applications)
        {
            const uint16 tlsPort = application->TlsPort;
            const uint16 port = application->Port;

            // init ssl Data for this port
            if (tlsPort != 0)
            {
                auto& options = Server.Options.ListenOptions.Emplace(Allocator);
                options.IsTls = true;
                options.Port = tlsPort;
                options.SslOptions.CertificateFile = application->CertificateFile;
                options.SslOptions.KeyFile = application->KeyFile;

                options.UseHttpServer();
            }

            // init default port
            if (port != 0)
            {
                auto& options = Server.Options.ListenOptions.Emplace(Allocator);
                options.Port = port;
                options.UseHttpServer();
            }

            LogInfo("Skylar") << "Content directory: " << *application->RootDirectory;
        }
    }

    void ModuleManager::UpdateApplications()
    {
        PROFILE_FUNCTION();

        // Applications settings list
        TArray< PoolApplicationSettings* > applications(Allocator);
        // Get full applications settings list
        GetAllApplicationSettings(applications);

        TArray<int32> updatedModules(Allocator);
        LogInfo("Skylar") << "Updating server applications..";

        for (const auto& application : applications)
        {
            const int32 moduleIndex = application->ModuleIndex;

            if (moduleIndex == INDEX_NONE)
            {
                LogWarning("Skylar") << "Module update skipped for " << application->ServerModuleUpdatePath;

                continue;
            }

            // If module is not updated (not checked)
            if (not updatedModules.Contains(moduleIndex))
            {
                // Check if update module is valid and loaded one isn't the same
                if (not application->ServerModuleUpdatePath.IsEmpty()
                        and application->ServerModuleUpdatePath != application->ServerModulePath)
                {
                    auto updateModuleStat = Neko::Platform::GetFileData(*application->ServerModuleUpdatePath);
                    auto updateModuleDate = updateModuleStat.ModificationTime;
                    int64 updateModuleSize = updateModuleStat.FileSize;

                    if (updateModuleStat.IsValid)
                    {
                        updateModuleStat = Neko::Platform::GetFileData(*application->ServerModulePath);
                        int64 moduleSize = updateModuleStat.FileSize;
                        auto moduleDate = updateModuleStat.ModificationTime;

                        auto& module = Modules[moduleIndex];

                        if (updateModuleStat.IsValid && (moduleSize != updateModuleSize or moduleDate < updateModuleDate))
                        {
                            UpdateApplication(module, applications, moduleIndex);
                        }
                    }
                }
                updatedModules.Push(moduleIndex);
            }
        }

        LogInfo("Skylar") << "Application modules have been updated.";

        Server.Controls.SetActiveFlag();
        Server.Controls.UpdateModulesEvent.Reset();
    }

    bool ModuleManager::UpdateApplication(Module& module, TArray<PoolApplicationSettings* >& applications, const uint32 index)
    {
        TArray<PoolApplicationSettings* > existing(Allocator);

        for (auto& application : applications)
        {
            // all apps with the same module
            if (application->ModuleIndex == index)
            {
                existing.Push(application);

                assert(application->OnApplicationExit != nullptr);
                application->OnApplicationExit();

                application->OnApplicationInit = std::function<bool(ApplicationInitContext)>();
                application->OnApplicationRequest = std::function<int16(Http::RequestData* , Http::ResponseData* )>();
                application->OnApplicationPostRequest = std::function<void(Http::ResponseData* )>();
                application->OnApplicationExit = std::function<void()>();
            }
        }

        module.Close();

        const auto application = *(existing.begin());
        const String& moduleName = application->ServerModulePath;

        const int32 directoryPos = moduleName.Find("/");
        const int32 extensionPos = moduleName.Find(".");

        String moduleNameNew(Allocator);

        if (extensionPos != INDEX_NONE and (directoryPos == INDEX_NONE or directoryPos < extensionPos))
        {
            moduleNameNew = moduleName.Mid(0, extensionPos);
            moduleNameNew += Math::RandGuid();
            moduleNameNew += moduleName.Mid(extensionPos);
        }
        else
        {
            moduleNameNew = moduleName;
            moduleNameNew += Math::RandGuid();
        }

        FileSystem::PlatformReadFile source;
        if (not source.Open(*application->ServerModuleUpdatePath))
        {
            LogError("Skylar") << "File \"" << *application->ServerModuleUpdatePath << "\" cannot be open.";

            return false;
        }

        FileSystem::PlatformWriteFile destination;
        if (not destination.Open(*moduleNameNew))
        {
            LogError("Skylar") << "File \"" << *moduleName << "\" cannot be open.";

            return false;
        }

        // rewrite a module file
        {
            TArray<uint8> data(Allocator);
            data.Resize(source.GetSize());
            source.Read(&data[0], source.GetSize());
            destination.Write(&data[0], data.GetSize());

            source.Close();
            destination.Close();
        }

        // Open updated module
        module.Open(moduleNameNew);

        if (not Neko::Platform::DeleteFile(*moduleName))
        {
            LogError("Skylar") << "File '" << *moduleName << "' can not be removed.";

            return false;
        }

        if (not Neko::Platform::MoveFile(*moduleNameNew, *moduleName))
        {
            LogError("Skylar") << "Module '" << *moduleNameNew << "' can not be renamed.";

            return false;
        }

        if (not module.IsOpen())
        {
            LogError("Skylar") << "Application module '" << *moduleName << "' can not be opened";

            return false;
        }

        // set application module methods
        bool success = SetApplicationModuleMethods(*application, module);
        if (not success)
        {
            return false;
        }

        for (auto& app : existing)
        {
            app->OnApplicationInit = application->OnApplicationInit;
            app->OnApplicationRequest = application->OnApplicationRequest;
            app->OnApplicationPostRequest = application->OnApplicationPostRequest;
            app->OnApplicationExit = application->OnApplicationExit;

            assert(app->OnApplicationInit);

            ApplicationInitContext items
            {
                *app->RootDirectory,

                &Allocator,
                &Server.FileSystem
            };
            app->OnApplicationInit(items);
        }

        return true;
    }

    void ModuleManager::Cleanup()
    {
        if (not List.IsEmpty())
        {
            TArray< PoolApplicationSettings* > applications(Allocator);

            for (auto app : applications)
            {
                assert(app->OnApplicationExit != nullptr);

                app->OnApplicationExit();

                NEKO_DELETE(Allocator, app) ;
            }

            applications.Clear();
        }

        if (not Modules.IsEmpty())
        {
            for (auto& module : Modules)
            {
                module.Close();
            }
            Modules.Clear();
        }
    }

#pragma region Load

    bool ModuleManager::LoadAppSettings(const String& fileName, TArray<Module>& modules)
    {
        LogInfo("Skylar") << "Loading application server settings";

        FileSystem::PlatformReadFile file;
        if (file.Open(*fileName) == false)
        {
            LogError("Skylar") << "Couldn't find " << *fileName << " serversettings file.";

            return false;
        }

        const auto tempDirectory = Neko::Platform::GetTempDirectory();
        const uint32 requestMaxSize = 10485760;

        // Create appsettings
        TArray< PoolApplicationSettings* > applicationSettingItems(Allocator);
        TArray< String > applicationNames(Allocator);

        // Load from json
        JsonDeserializer json(file, Path(*fileName), Allocator);;

        char label[255];

        json.DeserializeObjectBegin();
        json.DeserializeArrayBegin("applications");

        while (not json.IsArrayEnd())
        {
            auto* const settings = NEKO_NEW(Allocator, PoolApplicationSettings) ();
            applicationSettingItems.Push(settings);

            json.DeserializeObjectBegin();
            while (not json.IsObjectEnd())
            {
                json.DeserializeLabel(label, 255);

                if (EqualStrings(label, "application"))
                {
                    json.DeserializeObjectBegin();
                    json.Deserialize("name", applicationNames.Emplace(), "127.0.0.1");
                    json.Deserialize("rootDirectory", settings->RootDirectory.data, sizeof(settings->RootDirectory.data), "/");
                    json.Deserialize("tempDirectory", settings->TempDirectory.data, sizeof(settings->TempDirectory.data), "/");
                    json.Deserialize("module", settings->ServerModulePath, String::Empty);
                    json.Deserialize("moduleUpdate", settings->ServerModuleUpdatePath, String::Empty);
                    json.DeserializeObjectEnd();
                }
                else if (EqualStrings(label, "server"))
                {
                    json.DeserializeObjectBegin();
                    json.Deserialize("port", settings->Port, 4200);
                    json.Deserialize("tlsPort", settings->TlsPort, 0);
                    json.Deserialize("requestMaxSize", settings->RequestMaxSize, requestMaxSize);
                    json.DeserializeObjectEnd();
                }
                else if (EqualStrings(label, "ssl"))
                {
                    json.DeserializeObjectBegin();
                    json.Deserialize("certFile", settings->CertificateFile, String::Empty);
                    json.Deserialize("keyFile", settings->KeyFile, String::Empty);
                    json.DeserializeObjectEnd();
                }
                else
                {
                    // ...
                }
            }

            json.DeserializeObjectEnd();
        }
        json.DeserializeArrayEnd();
        json.DeserializeObjectEnd();

        file.Close();

        int32 applicationCount = applicationSettingItems.GetSize();
        LogInfo("Skylar") << "Found " << applicationCount << " application(s)";

        for (uint16 i = 0; i < applicationCount; ++i)
        {
            auto* settings = applicationSettingItems[i];
            const auto* rootDirectory = *settings->RootDirectory;

            // check directories
            if (not Platform::FileExists(rootDirectory))
            {
                LogWarning("Skylar") << "Set content directory does not exist, creating a new one at " << rootDirectory;
                if (not Platform::MakePath(rootDirectory, true))
                {
                    LogError("Skylar")
                        << "Couldn't create a content directory for the application \"" << applicationNames[i] << "\", "
                        << *Platform::GetLastErrorMessage();
                }
            }

            int16 moduleIndex = LoadModule(settings->ServerModulePath, rootDirectory, modules, *settings);
            bool success = moduleIndex!= -1;

            if (not success)
            {
                LogError("Skylar") << "Couldn't load the application module " << *settings->ServerModulePath;
                return false;
            }

            settings->ModuleIndex = moduleIndex;
            this->AddApplication(applicationNames[i], settings);
        }

        if (List.IsEmpty())
        {
            LogWarning("Skylar") << "Server does not contain any application.";

            return false;
        }

        return true;
    }

    bool ModuleManager::SetApplicationModuleMethods(PoolApplicationSettings& settings, Module& module)
    {
        assert(module.IsOpen());

        // todo more methods

        std::function<int(Http::RequestData* , Http::ResponseData* )> appRequestMethod;
        if (appRequestMethod = module.GetMethod<int16(*)(Http::RequestData* , Http::ResponseData* )>("OnApplicationRequest");
            appRequestMethod == nullptr)
        {
            return false;
        };

        std::function<void(Http::ResponseData* )> appPostRequestMethod;
        if (appPostRequestMethod = module.GetMethod<void(*)(Http::ResponseData* )>("OnApplicationPostRequest");
            appPostRequestMethod == nullptr)
        {
            return false;
        };

        std::function<bool(ApplicationInitContext)> appInitMethod;
        if (appInitMethod = module.GetMethod<bool(*)(ApplicationInitContext)>("OnApplicationInit");
            appInitMethod == nullptr)
        {
            return false;
        };

        std::function<void()> appExitMethod;
        if (appExitMethod = module.GetMethod<void(*)()>("OnApplicationExit");
            appExitMethod == nullptr)
        {
            return false;
        };

        settings.OnApplicationRequest = Neko::Move(appRequestMethod);
        settings.OnApplicationPostRequest = Neko::Move(appPostRequestMethod);
        settings.OnApplicationInit = Neko::Move(appInitMethod);
        settings.OnApplicationExit = Neko::Move(appExitMethod);

        return true;
    }

    int16 ModuleManager::LoadModule(const String& name, const char* rootDirectory, TArray<Module>& modules,
        PoolApplicationSettings& settings)
    {
        bool success = true;
        Module module(name);

        if (not module.IsOpen())
        {
            LogError("Skylar") << "Couldn't open \"" << *name << "\" application module.";

            return -1;
        }

        success = SetApplicationModuleMethods(settings, module);
        if (not success)
        {
            LogError("Skylar") << "One of the application methods is missing.";
            module.Close();

            return -1;
        }

        assert(settings.OnApplicationInit != nullptr);

        ApplicationInitContext items
        {
            rootDirectory,
            &Allocator,
            &FileSystem
        };

        success = settings.OnApplicationInit(items);
        if (not success)
        {
            LogWarning("Skylar") << "Application initialization returned unsuccessful result!";
            module.Close();

            return -1;
        }

        // Calculate module index
        int32 moduleIndex = INDEX_NONE;

        for (int32 i = 0; i < modules.GetSize(); ++i)
        {
            if (modules[i] == module)
            {
                moduleIndex = i;
                break;
            }
        }

        if (moduleIndex == INDEX_NONE)
        {
            moduleIndex = modules.GetSize();
            modules.Emplace(Neko::Move(module));
        }

        return moduleIndex;
    }

    void ModuleManager::GetAllApplicationSettings(TArray<PoolApplicationSettings* >& applications)
    {
        List.GetAllApplicationSettings(applications);
    }

    bool ModuleManager::AddApplication(const String& application, PoolApplicationSettings* settings)
    {
        LogInfo("Skylar") << "Added \"" << *application << "\" as an application";

        // Add application in server application list
        List.AddApplication(application, settings);

        return true;
    }

#pragma region end

#pragma region Application List

    ModuleManager::ApplicationsList::ApplicationsList(IAllocator& allocator)
        : Allocator(allocator)
        , List(allocator)
        , PoolApplicationSettings(nullptr)
    { }

    PoolApplicationSettings* ModuleManager::ApplicationsList::Find(const String& name) const
    {
        auto it = List.Find(Crc32(*name));

        return not it.IsValid() ? PoolApplicationSettings : it.value()->Find(name);
    }

    void ModuleManager::ApplicationsList::GetAllApplicationSettings(TArray< struct PoolApplicationSettings* >& applications) const
    {
        for (auto node : this->List)
        {
            const auto* subList = node;

            if (subList->PoolApplicationSettings != nullptr)
            {
                applications.Push(subList->PoolApplicationSettings);
            }

            subList->GetAllApplicationSettings(applications);
        }
    }

    void ModuleManager::ApplicationsList::AddApplication(const String& name, struct PoolApplicationSettings* settings)
    {
        ModuleManager::ApplicationsList* list = nullptr;

        if (auto it = List.Find(Crc32(*name)); it.IsValid())
        {
            LogWarning("Skylar") << "Attempt to add the application with existing name";
            // todo something?
            list = it.value();
        }
        else
        {
            list = NEKO_NEW(Allocator, ModuleManager::ApplicationsList)(Allocator);
            list->PoolApplicationSettings = settings;

            List.Insert(Neko::Crc32(*name), list);
        }

        //  list->AddApplication(name, settings);
    }

    void ModuleManager::ApplicationsList::Clear()
    {
        if (not List.IsEmpty())
        {
            for (auto it : List)
            {
                NEKO_DELETE(Allocator, it) ;
            }
            List.Clear();
        }
    }

#pragma region end
}
