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
//  Settings.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "../Engine/Core/Log.h"
#include "../Engine/Data/JsonSerializer.h"
#include "../Engine/FS/PlatformFile.h"
#include "../Engine/Platform/Platform.h"

#include "ContentTypes/ContentTypes.h"

#include "Settings.h"

namespace Neko
{
    namespace Skylar
    {
        ServerSettings::ServerSettings(FS::FileSystem& fileSystem, IAllocator& allocator)
        : Allocator(allocator)
        , FileSystem(fileSystem)
        , List(allocator)
        , SupportedMimeTypes(allocator)
        , ContentTypes(allocator)
        , ThreadsMaxCount(4)
        , MaxMemoryUsage(0)
        {
        }
        
        ServerSettings::~ServerSettings()
        {
            Clear();
        }
        
        void ServerSettings::AddContentType(IContentType& contentType)
        {
            LogInfo.log("Skylar") << "AddContentType: " << *contentType.GetName();
            ContentTypes.Insert(contentType.GetName(), &contentType);
        }
        
        bool ServerSettings::LoadAppSettings(const String& fileName, TArray<Module>& modules)
        {
            Neko::CPath path(*fileName);
            IStream* file = FileSystem.Open(FileSystem.GetDiskDevice(), path, FS::Mode::READ);
            
            if (file == nullptr)
            {
                LogError.log("Skylar") << "Couldn't find " << *fileName << " serversettings file.";
                
                return false;
            }
            
            const String tempDirectory = Neko::Platform::GetTempDirectory();
            const uint32 requestMaxSize = 10485760;
            const uint64 maxDefaultSize = Megabyte(64);
            
            // Create appsettings
            TArray< ApplicationSettings* > applicationSettingItems(Allocator);
            TArray< String > applicationNames(Allocator);
            
            // Load from json
            JsonSerializer json(*file, JsonSerializer::READ, path, Allocator);;
            
            char label[255];
            
            json.DeserializeObjectBegin();
            
            // shared
            json.DeserializeLabel(label, 255);
            json.DeserializeObjectBegin();
            {
                json.Deserialize("threadMaxCount", this->ThreadsMaxCount, 0);
                json.Deserialize("maxMemoryUsage", this->MaxMemoryUsage, maxDefaultSize);
            }
            json.DeserializeObjectEnd();
            
            json.DeserializeArrayBegin("applications");
            
            while (not json.IsArrayEnd())
            {
                ApplicationSettings* settings = NEKO_NEW(Allocator, ApplicationSettings) ();
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
            
            FileSystem.Close(*file);
            
            for (uint16 i = 0; i < applicationSettingItems.GetSize(); ++i)
            {
                auto* settings = applicationSettingItems[i];
                
                int16 moduleIndex = LoadModule(settings->ServerModulePath, settings->RootDirectory.data, modules, *settings);
                bool success = moduleIndex!= -1;
                
                if (not success)
                {
                    LogError.log("Skylar") << "Couldn't load module " << *settings->ServerModulePath;
                    return false;
                }
                
                settings->ModuleIndex = moduleIndex;
                
                this->AddApplication(applicationNames[i], settings);
            }
            
            if (Net::NetAddress address; address.Resolve(*applicationNames[0], Net::NA_UNSPEC))
            {
                this->ResolvedAddressString.Set(address.ToString());
                LogWarning.log("Skylar") << "Resolved address: " << *this->ResolvedAddressString;
            }
            else
            {
                LogWarning.log("Skylar") << "Couldn't resolve address of " << *applicationNames[0];
            }
            
            if (List.IsEmpty())
            {
                LogWarning.log("Skylar") << "Server does not contain any application";
                return false;
            }
            
            // Default mime types
            SupportedMimeTypes.Insert("html", "text/html");
            SupportedMimeTypes.Insert("js", "text/javascript");
            SupportedMimeTypes.Insert("css", "text/css");
            
            SupportedMimeTypes.Insert("gif", "image/gif");
            SupportedMimeTypes.Insert("jpg", "image/jpeg");
            SupportedMimeTypes.Insert("jpeg", "image/jpeg");
            SupportedMimeTypes.Insert("png", "image/png");
            
            SupportedMimeTypes.Insert("webm", "video/webm");
            SupportedMimeTypes.Insert("mp4", "video/mp4");
            SupportedMimeTypes.Insert("3gp", "video/3gp");
            
            // @todo More data content types (e.g. multipart/form-data).
            AddContentType(*NEKO_NEW(Allocator, TextPlain) (Allocator));
            AddContentType(*NEKO_NEW(Allocator, FormUrlencoded) (Allocator));
            AddContentType(*NEKO_NEW(Allocator, ApplicationJson) (Allocator));
            
            return true;
        }
        
        void ServerSettings::Clear()
        {
            if (not List.IsEmpty())
            {
                TArray< ApplicationSettings* > applications(Allocator);
                
                for (auto app : applications)
                {
                    assert(app->OnApplicationExit != nullptr);
                
                    app->OnApplicationExit();
                    
                    NEKO_DELETE(Allocator, app) ;
                }
                
                applications.Clear();
            }
            
            if (not ContentTypes.IsEmpty())
            {
                for (auto type : ContentTypes)
                {
                    NEKO_DELETE(Allocator, type) ;
                }
                
                ContentTypes.Clear();
            }
        }
        
        bool ServerSettings::SetApplicationModuleMethods(ApplicationSettings& settings, Module& module)
        {
            assert(module.IsOpen());
            
            // @todo get rid of std function
            
            std::function<int(Http::RequestData* , Http::ResponseData* )> appRequestMethod;
            appRequestMethod = module.GetMethod<int16(*)(Http::RequestData* , Http::ResponseData* )>("OnApplicationRequest");
            if (appRequestMethod == nullptr)
            {
                return false;
            };
            
            std::function<void(Http::ResponseData* )> appPostRequestMethod;
            appPostRequestMethod = module.GetMethod<void(*)(Http::ResponseData* )>("OnApplicationPostRequest");
            if (appPostRequestMethod == nullptr)
            {
                return false;
            };
            
            std::function<bool(ApplicationInitContext)> appInitMethod;
            appInitMethod = module.GetMethod<bool(*)(ApplicationInitContext)>("OnApplicationInit");
            if (appInitMethod == nullptr)
            {
                return false;
            };
            
            std::function<void()> appExitMethod;
            appExitMethod = module.GetMethod<void(*)()>("OnApplicationExit");
            if (appExitMethod == nullptr)
            {
                return false;
            };
            
            settings.OnApplicationRequest = Neko::Move(appRequestMethod);
            settings.OnApplicationPostRequest = Neko::Move(appPostRequestMethod);
            settings.OnApplicationInit = Neko::Move(appInitMethod);
            settings.OnApplicationExit = Neko::Move(appExitMethod);
            
            return true;
        }
        
        int16 ServerSettings::LoadModule(const String& name, const char* rootDirectory, TArray<Module>& modules, ApplicationSettings& settings)
        {
            bool success = true;
            Module module(name);
            
            if (not module.IsOpen())
            {
                LogError.log("Skylar") << "Couldn't open '" << *name << "' application module.";
                return -1;
            }
            
            success = SetApplicationModuleMethods(settings, module);
            if (not success)
            {
                LogError.log("Skylar") << "One of application methods is missing.";
                
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
            if (!success)
            {
                LogWarning.log("Skylar") << "Application initialization returned unsuccessful result!";
                
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
        
        void ServerSettings::GetAllApplicationSettings(TArray<ApplicationSettings* >& applications)
        {
            List.GetAllApplicationSettings(applications);
        }
        
        bool ServerSettings::AddApplication(const String& application, ApplicationSettings* settings)
        {
            // Add application in server application list
            List.AddApplication(application, settings);
            
            return true;
        }
        
        
        ServerSettings::ApplicationsList::ApplicationsList(IAllocator& allocator)
        : Allocator(allocator)
        , List(allocator)
        , ApplicationSettings(nullptr)
        {
            
        }
        
        ApplicationSettings* ServerSettings::ApplicationsList::Find(const String& name) const
        {
            auto it = List.Find(name);
            if (not it.IsValid())
            {
                return ApplicationSettings;
            }
            else
            {
                return it.value()->Find(name);
            }
            
            return nullptr;
        }
        
        void ServerSettings::ApplicationsList::GetAllApplicationSettings(TArray< struct ApplicationSettings* >& applications) const
        {
            for (auto node : List)
            {
                const ServerSettings::ApplicationsList* subList = node;
                
                if (subList->ApplicationSettings != nullptr)
                {
                    applications.Push(subList->ApplicationSettings);
                }
                
                subList->GetAllApplicationSettings(applications);
            }
        }
        
        void ServerSettings::ApplicationsList::AddApplication(const String& name, struct ApplicationSettings* settings)
        {
            ServerSettings::ApplicationsList* list = nullptr;
            
            if (auto it = List.Find(name); it.IsValid())
            {
                LogWarning.log("Skylar") << "Attempt to add the application with existing name";
                // @todo something?
                list = it.value();
            }
            else
            {
                list = NEKO_NEW(Allocator, ServerSettings::ApplicationsList)(Allocator);
                list->ApplicationSettings = settings;
                
                List.Insert(Neko::Move(name), list);
            }
            
            //  list->AddApplication(name, settings);
        }
        
        void ServerSettings::ApplicationsList::Clear()
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
    }
}

