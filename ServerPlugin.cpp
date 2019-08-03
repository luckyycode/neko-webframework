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

#include <vector>
#include "Plasma/Core/Actor.h"
#include "Plasma/ConsolePrintActor.h"

// Network base

namespace Neko
{
    using namespace Neko::Plasma;


    /*=============================================================================
     Messaging actor
     =============================================================================*/

    // The messaging actor takes a string as argument and a receiver of the composed
    // message. When it receives a message, i.e. an object, it will add its string
    // and forward the message to the next actor. It will report the actions to
    // the console. If the forwarding address is the Null address, it will print the
    // the combined message.
    //
    // It should be noted that the actor base class is 'virtual' and this is
    // strongly recommended to avoid the 'diamond inheritance problem' of object
    // oriented programming, see
    // https://en.wikipedia.org/wiki/Multiple_inheritance#The_diamond_problem

    class MessagingActor : public virtual Actor
    {
    public:

        // ---------------------------------------------------------------------------
        // The message
        // ---------------------------------------------------------------------------
        //
        // The message format supported by an actor is typically defined as a public
        // class since other actors communicating with this actor type will need to
        // send this type of message.
        //
        // In order to make the message more interesting, it will contain a standard
        // vector of standard strings, and an interface to operate on these strings.
        // It is defined complicated to show that a message can have any class format.

        class Message
        {
        private:

            std::vector< std::string > TheWisdoms;

        public:

            inline void AddPhrase( const std::string & TheWords )
            {
                TheWisdoms.push_back( TheWords );
            }

            inline void Print( std::ostream & Output )
            {
                std::ostream_iterator< std::string > OutStream( Output, " ");
                std::copy( TheWisdoms.begin(), TheWisdoms.end(), OutStream );
            }

            // There is one constructor that initiates an empty vector

            Message( void )
                    : TheWisdoms()
            { }

            // Then one that takes an initial string as the first phrase. Basically it
            // delegates the construction to the previous constructor and adds the
            // phrase using the above function

            Message( const std::string & TheWords )
                    : Message()
            {
                AddPhrase( TheWords );
            }

            // All messages must have a copy constructor to ensure that they are
            // properly stored at the destination actor even if the source actor
            // destroys the message. In this case it is easy to rely on the vector's
            // copy constructor.

            Message( const Message & OtherMessage )
                    : TheWisdoms( OtherMessage.TheWisdoms )
            { }
        };

        // ---------------------------------------------------------------------------
        // The actor state - data it protects and maintains
        // ---------------------------------------------------------------------------
        //
        // The actor will, as stated in the introduction, store a string that will
        // be added to the message, and then the combined string will be forwarded
        // to the next contributing actor, provided that this is not a Null address.

    private:

        std::string     MyWisdom;
        Address NextContributor;

        // ---------------------------------------------------------------------------
        // The message handler
        // ---------------------------------------------------------------------------
        //
        // Then a handler for this message type can be defined. It is a public
        // handler for this example since it will be called from the outside on the
        // first actor instantiation. In general one should not directly call a
        // method on an actor as it should respond to incoming messages, and it may
        // therefore be more common to declare the handlers as protected or private.

    public:

        void ForwardOrPrint( const Message & TheCommunication,
                const Address Sender      )
        {
            // First our own wisdom is written out

            ConsolePrint OutputMessage;

            OutputMessage << "My wisdom is \"" << MyWisdom << "\"" << std::endl;

            // Then the message is extended with our wisdom. It is necessary to start
            // from a copy of the message since the given message is passed as a
            // constant.

            Message AugmentedMessage( TheCommunication );

            AugmentedMessage.AddPhrase( MyWisdom );

            // If it is possible to route a message to the address stored as the
            // next contributor, it will be done, otherwise the message will just
            // be printed to the console print stream. Since the print function just
            // concatenates the strings it is necessary to add a new line at the end
            // to ensure that the output is correctly formatted.

            if ( NextContributor )
                Send( AugmentedMessage, NextContributor );
            else
            {
                AugmentedMessage.Print( OutputMessage );
                OutputMessage << std::endl;
            }
        }

        // ---------------------------------------------------------------------------
        // Constructor
        // ---------------------------------------------------------------------------
        //
        // The constructor takes the string to add and the next actor to process this
        // string. This information is just stored. Then it registers the handler
        // for the incoming message.

        MessagingActor( const std::string & WordsOfWisdom,
                const Address & NextActor = Address::Null() )
                : Actor(), MyWisdom( WordsOfWisdom ), NextContributor( NextActor )
        {
            RegisterHandler( this, &MessagingActor::ForwardOrPrint );
        }

    };


    // There is also a pointer to the execution framework of the console print
    // server to be used when the console print stream is used outside of an actor
    // and where it can be difficult to provide an execution framework.

    ConsolePrintServer * ConsolePrintServer::TheServer = nullptr;

    class ServerStartupTask : public Sync::Task
    {
    public:
        ServerStartupTask(IAllocator& allocator, FileSystem::IFileSystem& fileSystem)
            : Task(allocator)
            , FileSystem(fileSystem)
        {

          /*  SerializationInit();

            // The console print server is set to produce the output to the standard
            // output stream.
            ConsolePrintServer PrintServer(&std::cout, "ConsolePrintServer" );

            // Then the three actors producing the parts of the output. They are
            // constructed in inverse order so that the address of an actor already
            // constructed can be passed to the one that will forward the message to
            // it.
            MessagingActor FamousLastWords("World!", Address::Null() );
            MessagingActor TheHello( "Hello", FamousLastWords.GetAddress() );
            MessagingActor TheIntro( "Collectively we say:", TheHello.GetAddress() );

            // The message exchange is started by some event. In this case is is the
            // event that the first actor gets a message, and so it will be passed an
            // empty message from the null actor since the handler ignores the sender
            // address anyway.
            TheIntro.ForwardOrPrint( MessagingActor::Message(), Address::Null() );

            // Finally, it is just to wait for all messages to be handled by all actors
            // before main terminates
            Actor::WaitForGlobalTermination();*/
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


