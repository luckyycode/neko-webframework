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
//  TelegramApi.h
//  Neko Framework
//
//  Created by Neko on 6/29/18.
//

#pragma once

#include "../../Engine/Network/Http/Client.h"
#include "../../Engine/Utilities/NekoString.h"

namespace Neko
{
    using namespace Neko::Net;
    /// Provides api for Telegram services.
    class TelegramApi
    {
    public:
        
        TelegramApi(IAllocator& allocator);
        
        ~TelegramApi();
        
        /** Set a new bot token. Used for further requests. */
        void SetBotToken(const char* token);
        
        /** Sends bot request. Returns TRUE if succeeded. Bot token must be a valid value. */
        bool SendBotRequest(String method, const TArray<Http::HttpRequestParam>& parameters, String& response);
        
        bool SendRequest(const Net::Http::Url& url, const TArray<Http::HttpRequestParam>& parameters, String& response);
        
        void SendMessage(long chatId, String message, const char* parseMode = "");
        
        void SendSticker(long chatId, String id);
		
		void SendPhoto(long chatId, const char* filename, uint8* data, ulong size);
        
        void SetWebHook(const char* domain, const char* certificate = nullptr);
        
    private:
        
        //! Bot token.
        StaticString<64> BotToken;
        
        IAllocator& Allocator;
        
        //! Client ssl context.
        void* SslContext;
    };

}
