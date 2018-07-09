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
    namespace Http
    {
        ServerSettings::ServerSettings(FS::FileSystem& fileSystem, IAllocator& allocator)
        : Allocator(allocator)
        , FileSystem(fileSystem)
        , List(allocator)
        , SupportedMimeTypes(allocator)
        , ContentTypes(allocator)
        , ThreadsMaxCount(4)
        {
        }
        
        ServerSettings::~ServerSettings()
        {
            Clear();
        }
        
        void ServerSettings::AddContentType(IContentType* contentType)
        {
            GLogInfo.log("Http") << "AddContentType: " << *contentType->GetName();
            ContentTypes.Insert(contentType->GetName(), contentType);
        }
        
        bool ServerSettings::LoadAppSettings(const String& fileName, TArray<Module>& modules)
        {
            Neko::CPath path(*fileName);
            IStream* file = FileSystem.Open(FileSystem.GetDiskDevice(), path, FS::Mode::READ);
            
            if (file == nullptr)
            {
                GLogError.log("Http") << "Couldn't find " << *fileName << " appsettings file.";
                
                return false;
            }
            
            const String empty("", Allocator);
            const String tempDirectory = Neko::Platform::GetTempDirectory();
            const uint32 requestMaxSize = 10485760;
            
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
            }
            json.DeserializeObjectEnd();
            
            json.DeserializeArrayBegin("applications");
            
            while (!json.IsArrayEnd())
            {
                ApplicationSettings* settings = NEKO_NEW(Allocator, ApplicationSettings) ();
                applicationSettingItems.Push(settings);
                
                json.DeserializeObjectBegin();
                while (!json.IsObjectEnd())
                {
                    json.DeserializeLabel(label, 255);
                    
                    if (EqualStrings(label, "application"))
                    {
                        json.DeserializeObjectBegin();
                        json.Deserialize("name", applicationNames.Emplace(), "127.0.0.1");
                        json.Deserialize("rootDirectory", settings->RootDirectory, empty);
                        json.Deserialize("tempDirectory", settings->TempDirectory, tempDirectory);
                        json.Deserialize("module", settings->ServerModule, empty);
                        json.Deserialize("moduleUpdate", settings->ServerModuleUpdate, empty);
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
                        json.Deserialize("certFile", settings->CertificateFile, empty);
                        json.Deserialize("keyFile", settings->KeyFile, empty);
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
            
            for (uint32 i = 0; i < applicationSettingItems.GetSize(); ++i)
            {
                auto* settings = applicationSettingItems[i];
                
                int moduleIndex = LoadModule(settings->ServerModule, settings->RootDirectory, modules, *settings);
                bool success = moduleIndex!= -1;
                
                if (!success)
                {
                    GLogError.log("Http") << "Couldn't load module " << *settings->ServerModule;
                    return false;
                }
                
                settings->ModuleIndex = moduleIndex;
                
                this->AddApplication(applicationNames[i], settings);
            }
            
            Net::NetAddress address;
            address.Resolve(*applicationNames[0], Net::NA_IP);
            if (address.AddressType != Net::NA_BAD)
            {
                this->ResolvedAddressString.Set(address.ToString());
                GLogWarning.log("Http") << "Resolved address: " << *this->ResolvedAddressString;
            }
            else
            {
                GLogWarning.log("Http") << "Couldn't resolve address of " << *applicationNames[0];
            }
            
            if (List.IsEmpty())
            {
                GLogWarning.log("Http") << "Server does not contain any application";
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
            AddContentType(NEKO_NEW(Allocator, TextPlain) (Allocator));
            AddContentType(NEKO_NEW(Allocator, FormUrlencoded) (Allocator));
            AddContentType(NEKO_NEW(Allocator, ApplicationJson) (Allocator));
            
            return true;
        }
        
        void ServerSettings::Clear()
        {
            if (!List.IsEmpty())
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
            
            if (!ContentTypes.IsEmpty())
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
            
            bool success = true;
            
            // @todo get rid of std function
            
            std::function<int(Net::Http::RequestData* , Net::Http::ResponseData* )> appRequestMethod;
            appRequestMethod = module.GetMethod<int(*)(Net::Http::RequestData* , Net::Http::ResponseData* )>("OnApplicationRequest");
            success = appRequestMethod != nullptr;
            
            std::function<void(Net::Http::ResponseData* )> appPostRequestMethod;
            appPostRequestMethod = module.GetMethod<void(*)(Net::Http::ResponseData* )>("OnApplicationPostRequest");
            success = appPostRequestMethod != nullptr;
            
            std::function<bool(ApplicationInitDesc)> appInitMethod;
            appInitMethod = module.GetMethod<bool(*)(ApplicationInitDesc)>("OnApplicationInit");
            success = appInitMethod != nullptr;
            
            std::function<void()> appExitMethod;
            appExitMethod = module.GetMethod<void(*)()>("OnApplicationExit");
            success = appExitMethod != nullptr;
            
            settings.OnApplicationRequest = Neko::Move(appRequestMethod);
            settings.OnApplicationPostRequest = Neko::Move(appPostRequestMethod);
            settings.OnApplicationInit = Neko::Move(appInitMethod);
            settings.OnApplicationExit = Neko::Move(appExitMethod);
            
            return success;
        }
        
        int32 ServerSettings::LoadModule(const String& name, const String& rootDirectory, TArray<Module>& modules, ApplicationSettings& settings)
        {
            bool success = true;
            Module module(name);
            
            if (!module.IsOpen())
            {
                GLogError.log("Http") << "Couldn't open '" << *name << "' application module";
                return false;
            }
            
            success = SetApplicationModuleMethods(settings, module);
            
            assert(settings.OnApplicationInit != nullptr);
            
            ApplicationInitDesc items
            {
                *rootDirectory
            };
            success = settings.OnApplicationInit(items);
            
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
        
        bool ServerSettings::AddApplication(const String& application, ApplicationSettings* settings)
        {
            // Add application in server application list
            List.AddApplication(application, settings);
            
            return true;
        }
        
        
        ServerSettings::ApplicationsList::ApplicationsList(IAllocator& allocator)
        : Allocator(allocator)
        , List(allocator)
        {
            
        }
        
        ApplicationSettings* ServerSettings::ApplicationsList::Find(const String& name) const
        {
            auto it = List.Find(name);
            if (!it.IsValid())
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
            
            auto it = List.Find(name);
            
            if (it.IsValid())
            {
                GLogWarning.log("Http") << "Attempt to add the application with existing name";
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
            if (!List.IsEmpty())
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

