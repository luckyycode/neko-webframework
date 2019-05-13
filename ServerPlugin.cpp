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
//  Network.cpp
//  Neko SDK
//
//  Created by Neko Vision on 11/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "Engine/Core/Engine.h"
#include "Engine/Core/Log.h"
#include "Engine/Utilities/Utilities.h"
#include "Engine/Utilities/CommandLineParser.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Core/IPlugin.h"
#include "Engine/Mt/Task.h"

#include "Skylar/Server.h"
#include "Clustering/Mako.h"

// Network base

namespace Neko
{
    class ServerStartupTask : public Sync::Task
    {
    public:
        ServerStartupTask(IAllocator& allocator, FileSystem::IFileSystem& fileSystem)
            : Task(allocator)
            , FileSystem(fileSystem)
        {
        }
        
        virtual int32 DoTask() override
        {
            bool forceStart = false;
            char serverName[64];

            CopyString(serverName, Skylar::DEFAULT_SERVER_NAME);
            {
                uint64 maxMemory = 1024 * 1024 * 128; // in mb
                char commandLine[2048];
                Platform::GetSystemCommandLine(commandLine, Neko::lengthOf(commandLine));
                
                CommandLineParser parser(commandLine);
                while (parser.Next())
                {
                    if (parser.CurrentEquals("--force"))
                    {
                        forceStart = true;
                    }
                    else if (parser.CurrentEquals("--name"))
                    {
                        if (not parser.Next())
                        {
                            break;
                        }
                        
                        parser.GetCurrent(serverName, Neko::lengthOf(serverName));
                    }
                    else if (parser.CurrentEquals("--maxmemory"))
                    {
                        if (not parser.Next())
                        {
                            break;
                        }
                        
                        char memory[8];
                        parser.GetCurrent(memory, Neko::lengthOf(memory));
                        maxMemory = StringToUnsignedLong(memory);
                    }

                    if (not parser.Next())
                    {
                        break;
                    }
                }
            }
            
            Skylar::Server server(GetAllocator(), FileSystem);
            String mutableName(serverName);

            return server.StartCommand(mutableName, forceStart);
        }
        
    private:
        FileSystem::IFileSystem& FileSystem;
    };
    
    class Network : public IPlugin
    {
    public:
        explicit Network(IEngine& engine)
            : Engine(engine)
            , Allocator(engine.GetAllocator())
        {
            if (this->Task = NEKO_NEW(Allocator, ServerStartupTask)(Allocator, engine.GetFileSystem());
                not this->Task->Create("Skylar task"))
            {
                assert(false);
            }
        }
        
        virtual ~Network()
        {
            this->Task->Destroy();
            
            NEKO_DELETE(Allocator, this->Task);
            this->Task = nullptr;
        }
        
    public:
        NEKO_FORCE_INLINE const char* GetName() const override { return "skylar"; }
      
        void Update(float fDelta) override { }

    private:
        ServerStartupTask* Task;
        
        Neko::IAllocator& Allocator;
        IEngine& Engine;
    };
}

NEKO_PLUGIN_ENTRY(httpserver)
{
    return NEKO_NEW(engine.GetAllocator(), Neko::Network)(engine);
}


