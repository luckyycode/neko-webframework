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
    /// Provides api for Telegram services.
    class TelegramApi
    {
    public:
        
        TelegramApi(IAllocator& allocator);
        
        ~TelegramApi();
        
        void SetBotToken(const char* token);
        
        bool SendBotRequest(String method, const TArray<Net::Http::HttpRequestParam>& parameters, String& response);
        
        bool SendRequest(const Net::Http::Url& url, const TArray<Net::Http::HttpRequestParam>& parameters, String& response);
        
        void SendMessage(long chatId, String message, const char* parseMode = "");
        
        void SendSticker(long chatId, String id);
		
		void SendPhoto(long chatId, const char* filename, uint8* data, ulong size);
        
        void SetWebHook(const char* domain, const char* certificate = nullptr);
        
    private:
        
        //! Bot token.
        String BotToken;
        
        IAllocator& Allocator;
        
        //! Client ssl context.
        void* SslContext;
    };

}
