#pragma once

namespace Neko
{
    namespace Nova
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
            
            bool AutoIdRenewal : 1;
            
        };
        
        // Shared options.
        struct Options
        {
            template<typename TFunc>
            inline Options& ConfigureSession(TFunc func)
            {
                func(this->Session);
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
