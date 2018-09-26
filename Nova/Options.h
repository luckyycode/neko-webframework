#pragma once

#include "Conventions/Enums/SessionCookieType.h"

namespace Neko
{
    namespace Nova
    {
        /// Identity session options.
        struct SessionOptions
        {
            // Name of the cookie to use.
            String Name;
            
            String CookiePath;
            
            String Secret;
            
            String CsrfKey;
            
            uint32 Lifetime;
            
            SessionStorageType StorageType;
            
            bool CsrfProtectionEnabled : 1;
            
            // Skip X-FRAME-OPTIONS header.
            bool SuppressXFrameOptionsHeader : 1;
            
            bool AutoIdRenewal : 1;
            
        };
        
        // Shared options.
        struct Options
        {
            template<typename TFunc>
            inline Options& ConfigureSession(TFunc func)
            {
                // imitate expressions lol
                func(this->Session);
                return *this;
            }
            
            static Options& Instance();
            static const SessionOptions& SessionOptions();
            
        public:
            
            struct SessionOptions Session;
        };
        
        inline Options& Options::Instance()
        {
            static Options options;
            return options;
        }
        
        inline const SessionOptions& Options::SessionOptions()
        {
            return Instance().Session;
        }
    }
}
