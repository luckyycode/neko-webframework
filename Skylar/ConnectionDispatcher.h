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
//  ConnectionDispatcher.h
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#pragma once

#include "Server.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Debug.h"
#include "Engine/Core/Async.h"
#include "Engine/Core/Profiler.h"
#include "Engine/Mt/Task.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Platform/SocketPooler.h"
#include "Engine/Network/SocketPool.h"
#include "Engine/FileSystem/PlatformFile.h"

#include "Server.h"
#include "Http.h"
#include "ISsl.h"
#include "Protocol.h"

#include "ConnectionManager.h"
#include "ListenOptions.h"

#include "../Sockets/SocketSSL.h"
#include "../Sockets/SocketDefault.h"

namespace Neko::Skylar
{
    struct SkylarConnection;
    
    /** Handler to process incoming requests. @see Server::ProcessWorkerThreads */
    class ConnectionDispatcher
    {
    public:
        ConnectionDispatcher(Server& server, Net::SocketPool& sockets, Sync::Event* event, MiddlewareDelegate& delegate);
        ~ConnectionDispatcher();

        IProtocol* CreateProto(SkylarConnection& connection, IAllocator& allocator) const;
        bool InstantiateProtocolFromConnection(SkylarConnection& connection) const;

        bool Handle();
        void Complete();

    private:
        friend struct Job;
        
        ConnectionManager ConnectionManager;
        
        /** Last connection id */
        static Sync::ThreadSafeCounter64 LastConnectionId;

        /** Amount of active requests. */
        mutable Sync::ThreadSafeCounter ConcurrentRequestCount;

        // temp
        ProtocolOptions Options;

        MiddlewareDelegate& Delegate;
        //! Used for various timings
        Timer Timer;
        
        Server& Server;
        CycleManager& Controls;
        ModuleManager& Modules;

        Net::SocketPool& Sockets;
        Sync::Event& QueueNotFullEvent;

        IAllocator& Allocator;

        ISsl* Ssl;

        Sync::Event* Event;
    };

}


