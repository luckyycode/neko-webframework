#pragma once

namespace Neko
{
    namespace Mvc
    {
        /// Identity session options.
        struct SessionOptions
        {
            String Name;
            
            String CookiePath;
            String StorageType;
            
            String Secret;
            
            String CsrfKey;
            
            uint32 Lifetime;
            
            uint16 MaxGcLifetime;
            
            uint16 GcProbability;
            
            bool IsCsrfProtectionEnabled : 1;
            
            bool SuppressXFrameOptionsHeader : 1;
            
            bool AutoIdRegeneration : 1;
            
        };
        
        // Shared options.
        struct Options
        {
            template<typename TFunc>
            inline void Configure(TFunc func)
            {
                func(*this);
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
