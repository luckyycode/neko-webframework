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
//  RequestPoolHandler.h
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

#include "../Sockets/SocketSSL.h"
#include "../Sockets/SocketDefault.h"

namespace Neko::Skylar
{
    /** Handler to process incoming requests. @see Server::ProcessWorkerThreads */
    class RequestPoolHandler
    {
    public:
        RequestPoolHandler(Server& server, Net::SocketPool& sockets, MT::Event* event);

        IProtocol* CreateProto(ISocket& socket, void* stream, IAllocator& allocator) const;
        void InstantiateProtocolFor(ISocket &socket, void *stream) const;

        bool Handle();

    private:
        friend struct Job;

        /** Amount of active requests. */
        mutable MT::ThreadSafeCounter ConcurrentRequestCount;

        Server& Server;
        CycleManager& Controls;
        ServerSharedSettings& Settings;

        Net::SocketPool& Sockets;

        MT::Event& QueueNotFullEvent;

        IAllocator& Allocator;

        ISsl* Ssl;

        MT::Event* Event;
    };

}


