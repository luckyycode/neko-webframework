//
//  TelegramApi.h
//  HttpServer
//
//  Created by Neko on 6/29/18.
//

#pragma once

#include "../../Engine/Network/Http/Client.h"
#include "../../Engine/Utilities/NekoString.h"

namespace Neko
{
    class TelegramApi
    {
    public:
        
        TelegramApi(IAllocator& allocator);
        
        ~TelegramApi();
        
        void SetBotToken(const char* token);
        
        bool SendBotRequest(String method, const TArray<Net::Http::HttpRequestParam>& parameters, String& response);
        
        bool SendRequest(const Net::Http::Url& url, const TArray<Net::Http::HttpRequestParam>& parameters, String& response);
        
        void SendMessage(long chatId, String message);
        
        void SetWebHook(const char* domain, const char* certificate = nullptr);
        
    private:
        
        //! Bot token.
        String BotToken;
        
        IAllocator& Allocator;
        
        //! Client ssl context.
        void* SslContext;
    };

}
