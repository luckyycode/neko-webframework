//
//          *                  *
//             __                *
//           ,db'    *     *
//          ,d8/       *        *    *
//          888
//          `db\       *     *
//            `o`_                    **
//         *               *   *    _      *
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
//    _   _        _                                _
//   | \ | |  ___ | | __ ___     ___  _ __    __ _ (_) _ __    ___
//   |  \| | / _ \| |/ // _ \   / _ \| '_ \  / _` || || '_ \  / _ \
//   | |\  ||  __/|   <| (_) | |  __/| | | || (_| || || | | ||  __/
//   |_| \_| \___||_|\_\\___/   \___||_| |_| \__, ||_||_| |_| \___|
//                                           |___/
//  Network.cpp
//  Neko engine
//
//  Created by Neko Vision on 11/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "../Engine/Network/Network.h"
#include "../Engine/Core/Engine.h"
#include "../Engine/Core/Profiler.h"
#include "../Engine/Core/Log.h"
#include "../Engine/Utilities/Utilities.h"
#include "../Engine/Utilities/Timer.h"
#include "../Engine/Utilities/Templates.h"
#include "../Engine/Utilities/CommandLineParser.h"
#include "../Engine/Platform/Platform.h"
#include "../Engine/Core/IPlugin.h"
#include "../Engine/Mt/Task.h"


#include "../Engine/Network/Http/Url.h"
#include "../Engine/Network/Http/Client.h"
#include "../Engine/Network/Http/Extensions/Extensions.h"
#include "../Engine/Network/NetSocket.h"
#include "../Engine/Network/NetSocketV6.h"

#include "Server/IProtocol.h"
#include "SocketDefault.h"
#include "SocketSSL.h"

#include "Utils.h"

#include "Server/Server.h"
#include "SampleModule/TelegramApi.h"

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
            char serverName[64] = { "Nekoo" };
            
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
            ServerTask* task = NEKO_NEW(Allocator, ServerTask)(Allocator, engine.GetFileSystem());
            if (!task->Create("Neko Server"))
            {
                assert(false);
            }
        }
        
        virtual ~Network()
        {
        }
        
    public:
        
        NEKO_FORCE_INLINE const char* GetName() const override { return "httpserver"; }
      
        void Update(float fDelta) override { }

    private:
        
        Neko::IAllocator& Allocator;
        IEngine& Engine;
    };
}

NEKO_PLUGIN_ENTRY(httpserver)
{
    return NEKO_NEW(engine.GetAllocator(), Neko::Network)(engine);
}


