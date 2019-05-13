#include "SharedSettings.h"

namespace Neko::Skylar
{
    /**
     * Pure interface capable for any type of a connection.
     * A protocol is created per connection/switch.
     */
    class IProtocol
    {
    public:
        /** Processes the request. */
        virtual IProtocol* Process() = 0;

        virtual void SetSettingsSource(const ServerSharedSettings &settings) = 0;

        /** Closes this protocol. */
        virtual void Close() = 0;

        /** Sends data by net socket. */
        virtual long SendData(const void* source, ulong size,
            const int32& timeout, Http::DataCounter* dataCounter) const = 0;
    };
}