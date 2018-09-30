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
//  SocketSSL.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "SocketSSL.h"

#if USE_OPENSSL
#   include <openssl/opensslv.h>

namespace Neko::Skylar
{
    SocketSSL::SocketSSL(const Net::INetSocket& socket, SSL_CTX* context)
    : Socket(socket)
    {
        assert(context != nullptr);
        
        // create an SSL connection and attach it to the socket
        this->Connection = SSL_new(context);
        SSL_set_fd(this->Connection, socket.GetNativeHandle());
    }
    
    SocketSSL::SocketSSL(const Net::INetSocket& socket, SSL* connection)
    : Connection(connection)
    , Socket(socket)
    {
        assert(connection != nullptr);
    }
    
    int32 SocketSSL::Connect()
    {
        int32 result = SSL_connect(this->Connection);
        return result;
    }
    
    long SocketSSL::GetPacketBlocking(void* buffer, const ulong length, const int32& timeout) const
    {
        long result;
        int32 innerError;
        
        Net::INetSocket socket;
        socket.Init(this->GetNativeHandle(), Net::ESocketType::TCP);
        
        do
        {
            if (not socket.WaitForDataInternal(timeout, false))
            {
                // timeout
                result = -1;
                break;
            }
            
            result = ::SSL_read(this->Connection, buffer, length);
            innerError = SSL_get_error(this->Connection, result);
        }
        while (innerError == SSL_ERROR_WANT_READ or innerError == SSL_ERROR_WANT_WRITE);
        
        return result;
    }
    
    long SocketSSL::SendAllPacketsWait(const void* buffer, const ulong length, const int32& timeout) const
    {
        ulong checkSize = length;
        
        if (checkSize == 0)
        {
            return -1;
        }
        
        ulong total = 0;
        int32 innerError = 0;
        
        Net::INetSocket socket;
        socket.Init(this->GetNativeHandle(), Net::ESocketType::TCP);
        
        while (total < length)
        {
            if (checkSize > length - total)
            {
                checkSize = length - total;
            }
            
            long result = 0;
            
            do
            {
                // Wait for send all data to client
                if (not socket.WaitForAnyDataInternal(-1, true))
                {
                    continue;
                }
                
                result = ::SSL_write(this->Connection, reinterpret_cast<const uint8_t *>(buffer) + total, checkSize);
                innerError = ::SSL_get_error(this->Connection, result);
            }
            while (innerError == SSL_ERROR_WANT_WRITE);
            
            total += result;
        }
        
        return total;
    }
    
    bool SocketSSL::Handshake()
    {
        int32 innerResult = 0;
        int32 result = 0;
        
        do
        {
            if (not Socket.WaitForAnyDataInternal(-1, false))
            {
                continue;
            }
            
            result = SSL_accept(this->Connection);
            innerResult = SSL_get_error(this->Connection, result);
        }
        while (innerResult == SSL_ERROR_WANT_READ);
        
        // uhh
        //            do
        //            {
        //                result = SSL_do_handshake(this->Connection);
        //                innerResult = SSL_get_error(this->Connection, result);
        //            }
        //            while (innerResult == SSL_ERROR_WANT_READ || innerResult == SSL_ERROR_WANT_WRITE);
        
        return true;
    }
    
    void SocketSSL::Close()
    {
        // Wait for send all data to client
        Socket.WaitForAnyDataInternal(-1, true);
        
        SSL_shutdown(this->Connection);
        Socket.Close();
    }
}

#endif


