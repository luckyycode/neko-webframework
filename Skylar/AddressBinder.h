#pragma once

#include <Engine/Platform/SystemShared.h>
#include <Engine/Containers/Array.h>
#include <Engine/Network/NetSocketBase.h>
#include <functional>
#include "ListenOptions.h"
#include "SkylarOptions.h"

namespace Neko::Skylar
{
    class ISsl;
    using OnBindCallback = std::function<void (ListenOptions&, Net::NetSocketBase*)>;

    /** Address binder, binds address/port for each option. */
    class AddressBinder
    {
    public:
        AddressBinder(IAllocator& allocator);
        ~AddressBinder();

        void Bind(SkylarOptions& options, OnBindCallback onBind);

        void Destroy();
        void Clear();

        ISsl* Ssl;

    private:
        /**
         * Binds a socket port to the address.
         *
         * @param port  Port to bound address to.
         * @return TRUE if succeeded, FALSE otherwise.
         */
        Net::NetSocketBase* BindPort(const uint16 port);

    private:
        TArray<uint16> Ports;
        IAllocator& Allocator;
    };
}
