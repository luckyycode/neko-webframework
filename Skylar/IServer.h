#pragma once

namespace Neko::Skylar
{
    /** Server interface. */
    class IServer
    {
    public:
        virtual ~IServer() { };

        /** Initializes the server. */
        virtual bool Init() = 0;
        /** Shuts down the server. */
        virtual void Shutdown() = 0;

        virtual uint16 Run() = 0;

        virtual uint32 GetServerProcessId(const String& serverName) const = 0;
        virtual class IAllocator& GetAllocator() = 0;
        
        virtual const char* GetServerRole() const = 0;
    };
}
