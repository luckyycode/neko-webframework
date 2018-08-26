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
//  OpenSsl.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "ISsl.h"
#include <openssl/ssl.h>

#include "Engine/Core/Log.h"
#include "../ApplicationSettings.h"

namespace Neko
{
    namespace Skylar
    {
# if USE_OPENSSL
        class OpenSslImpl : public ISsl
        {
        public:
            
            OpenSslImpl(IAllocator& allocator)
            : TlsData(allocator)
            { }
            
            bool Init() override
            {
                // initialize OpenSSL
                ::SSL_load_error_strings ();
                ::SSL_library_init (); // @note SSL_library_init() always returns "1", so it is safe to discard the return value.
                
                return true;
            }
            
            /**
             * Called by OpenSSL when a client sends an ALPN request to check if protocol is supported.
             * The server later will check a type and decide which protocol to use.
             */
            static int AlpnCallback(SSL* session, const Byte** out, Byte* outLength, const Byte* in, uint32 inLength, void* arg)
            {
                for (uint32 i = 0; i < inLength; i += in[i] + 1)
                {
                    auto protocol = String((const char* ) &in[i + 1]).Mid(0, in[i]);
                    
                    if (StartsWith(*protocol, "h2"))
                    {
                        *out = (Byte* ) in + i + 1;
                        *outLength = in[i];
                        
                        // success
                        return SSL_TLSEXT_ERR_OK;
                    }
                }
                
                // noop, fallback to http/1.1
                return SSL_TLSEXT_ERR_NOACK;
            }
            
            void* InitSsl(const ApplicationSettings& application) override
            {
                SSL_CTX* context = nullptr;
                
                const auto& certificate = application.CertificateFile;
                const auto& privateKey = application.KeyFile;
                
                LogInfo.log("Skylar") << "Configuring ssl configuration for the application..";
                
                context = SSL_CTX_new(SSLv23_server_method());
                
                if (SSL_CTX_use_certificate_file(context, *certificate, SSL_FILETYPE_PEM) <= 0)
                {
                    LogError.log("Skylar") << "Couldn't load SSL certificate..  ";
                    goto cleanupSsl;
                }
                
                if (SSL_CTX_use_PrivateKey_file(context, privateKey.IsEmpty() ? *certificate : *privateKey, SSL_FILETYPE_PEM) <= 0)
                {
                    LogError.log("Skylar") << "Couldn't load SSL private key (or certificate pair)..";
                    goto cleanupSsl;
                }
                
                if (not SSL_CTX_check_private_key(context))
                {
                    LogError.log("Skylar") << "Couldn't verify SSL private key!";
                    goto cleanupSsl;
                }
                
                // SSL_CTX_set_alpn_select_cb(context, AlpnCallback, nullptr);
                
                return context;
                
            cleanupSsl:
                {
                    SSL_CTX_free(context);
                    return nullptr;
                }
            }
            
            bool NegotiateProtocol(void* session, String& protocol) override
            {
                const Byte* proto = nullptr;
                uint32 length = 0;
                
                // ALPN
                SSL_get0_alpn_selected(static_cast<SSL* >(session), &proto, &length);
                if (length == 0)
                {
                    // NPN
                    SSL_get0_next_proto_negotiated(static_cast<SSL* >(session), &proto, &length);
                }
                
                bool ok = length > 0;
                if (ok)
                {
                    protocol.Assign((const char*)proto, length);
                }
                
                return ok;
            }
            
            void Clear() override
            {
                if (not TlsData.IsEmpty())
                {
                    for (auto& item : TlsData)
                    {
                        SSL_CTX_free(static_cast<SSL_CTX* >(item));
                    }
                    TlsData.Clear();
                }
                
            }
            
        public:
            
            void AddSession(uint16 port, void* context) override
            {
                this->TlsData.Insert(port, context);
            }
            
            const TlsMap& GetTlsData() const override
            {
                return this->TlsData;
            }
            
        private:
            
            //! Ssl contexts map.
            TlsMap TlsData;
        };
        
        ISsl* ISsl::Create(IAllocator& allocator)
        {
            static OpenSslImpl impl(allocator);
            return static_cast<ISsl* >(&impl);
        }
        
        void ISsl::Destroy(ISsl& ssl)
        {
            auto impl = static_cast<OpenSslImpl& >(ssl);
            impl.Clear();
        }
# endif
        
    }
}

