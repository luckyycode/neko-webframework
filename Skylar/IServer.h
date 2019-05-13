
namespace Neko::Skylar
{
    /** Server interface. */
    class IServer
    {
    public:
        virtual ~IServer() { };

        /**
         * Binds a socket port to the address.
         *
         * @param port  Port to bound address to.
         * @param ports Empty array of ports.
         * @return TRUE if succeeded, FALSE otherwise.
         */
        virtual bool BindPort(const uint16 port, TArray<uint16>& ports) = 0;

        /** Initializes the server. */
        virtual bool Init() = 0;
        /** Shuts down the server. */
        virtual void Shutdown() = 0;

        virtual uint16 Run() = 0;

        virtual uint32 GetServerProcessId(const String& serverName) const = 0;
        virtual class IAllocator& GetAllocator() = 0;
    };
}