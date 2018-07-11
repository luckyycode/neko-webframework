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
//  Network.cpp
//  Neko engine
//
//  Created by Neko Vision on 11/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "../Engine/Core/Engine.h"
#include "../Engine/Core/Log.h"
#include "../Engine/Utilities/Utilities.h"
#include "../Engine/Utilities/CommandLineParser.h"
#include "../Engine/Platform/Platform.h"
#include "../Engine/Core/IPlugin.h"
#include "../Engine/Mt/Task.h"

#include "Server/Server.h"

// Network base

namespace Neko
{
    static Http::Server* ServerInstance = nullptr;
    
    class ServerTask : public MT::Task
    {
    public:
        
        ServerTask(IAllocator& allocator, FS::FileSystem& fileSystem)
        : Task(allocator)
        , FileSystem(fileSystem)
        {
        }
        
        virtual int32 DoTask() override
        {
            bool forceStart = false;
            char serverName[64] = { DEFAULT_SERVER_NAME };
            
            char commandLine[2048];
            Platform::GetSystemCommandLine(commandLine, Neko::lengthOf(commandLine));
            
            CCommandLineParser parser(commandLine);
            while (parser.Next())
            {
                if (parser.CurrentEquals("--force"))
                {
                    forceStart = true;
                }
                else if (parser.CurrentEquals("--name"))
                {
                    if (!parser.Next())
                    {
                        break;
                    }
                    
                    parser.GetCurrent(serverName, Neko::lengthOf(serverName));
                }
                
                if (!parser.Next())
                {
                    break;
                }
            }
            
            ServerInstance = NEKO_NEW(GetAllocator(), Http::Server )(GetAllocator(), FileSystem);
            int32 exitCode = ServerInstance->StartCommand(serverName, forceStart);
            
            NEKO_DELETE(GetAllocator(), ServerInstance);
            return exitCode;
        }
        
    private:
        
        FS::FileSystem& FileSystem;
    };
    
    class Network : public IPlugin
    {
    public:
        
        Network(IEngine& engine)
        : Engine(engine)
        , Allocator(engine.GetAllocator())
        {
            this->Task = NEKO_NEW(Allocator, ServerTask)(Allocator, engine.GetFileSystem());
            if (!this->Task->Create("Neko Server"))
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
        
        NEKO_FORCE_INLINE const char* GetName() const override { return "httpserver"; }
      
        void Update(float fDelta) override { }

    private:
        
        ServerTask* Task;
        
        Neko::IAllocator& Allocator;
        IEngine& Engine;
    };
}

NEKO_PLUGIN_ENTRY(httpserver)
{
    return NEKO_NEW(engine.GetAllocator(), Neko::Network)(engine);
}


