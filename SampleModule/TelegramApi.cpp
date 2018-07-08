//
//  TelegramApi.cpp
//  HttpServer
//
//  Created by Neko on 6/29/18.
//

#include "../../Engine/Network/Http/Url.h"
#include "../../Engine/Network/NetSocket.h"
#include "../../Engine/Core/Log.h"

#include "TelegramApi.h"

#include "../SocketSSL.h"

namespace Neko
{
    static const char* TelegramApiEndpoint = "https://api.telegram.org";
    static const char* TelegramBotApiEndpoint = "https://api.telegram.org/bot";
    static const uint32 TelegramApiPort = 443;
    
    
    TelegramApi::TelegramApi(IAllocator& allocator)
    : Allocator(allocator)
    {
        // client context
        this->SslContext = SSL_CTX_new (::SSLv23_client_method());
    }
    
    TelegramApi::~TelegramApi()
    {
        assert(this->SslContext != nullptr);
        SSL_CTX_free((SSL_CTX* )this->SslContext);
    }
  
    bool TelegramApi::SendRequest(const Net::Http::Url& url, const TArray<Net::Http::HttpRequestParam>& parameters, String& response)
    {
        GLogInfo.log("Telegram") << "Sending request " << *url.Host;
        
        Net::INetSocket socket;
        socket.Init(*url.Host, TelegramApiPort, Net::ESocketType::TCP);
        
        socket.MakeNonBlocking(false);
        socket.SetSocketStreamNoDelay();
        
        // connect using inner socket
        int16 result = socket.Connect();
        
        if (result < 0)
        {
            GLogError.log("Telegram") << "SendRequest failed to connect";
            return false;
        }
        
        Http::SocketSSL socketSsl(socket, (SSL_CTX* )SslContext);
        
        // connect using ssl
        result = socketSsl.Connect();
        if (result < 0)
        {
            GLogError.log("Telegram") << "SendRequest failed to connect via SSL";
            return false;
        }
        
        const String requestText = Net::Http::Client::GenerateRequest(url, parameters, false);
        
        long bytes = socketSsl.SendAllPacketsWait(*requestText, requestText.Length(), -1);
        if (bytes < 0)
        {
            GLogError.log("Http") << "Couldn't send request";
            return false;
        }
        
        bytes = socketSsl.GetPacketBlocking((void* )response.c_str(), response.Length(), -1);

        if (bytes <= 0)
        {
            GLogWarning.log("Telegram") << "SendRequest couldn't get response back, hmm";
            return true;
        }
        
        socketSsl.Close();
        
        return true;
    }
    
    void TelegramApi::SendMessage(long chatId, String message)
    {
        TArray<Net::Http::HttpRequestParam> args(Allocator);
        
        args.Reserve(2);
        args.Emplace("chat_id", (int64)chatId);
        args.Emplace("text", *message);
        
        String response(Allocator);
        response.Resize(1024);
        
        SendBotRequest("sendMessage", args, response);
    }
    
    void TelegramApi::SetBotToken(const char* token)
    {
        this->BotToken = String(token, Allocator);
    }
    
    void TelegramApi::SetWebHook(const char* domain, const char* certificate)
    {
        TArray<Net::Http::HttpRequestParam> args(Allocator);
        args.Reserve(2);
        args.Emplace("url", domain);
        if (certificate != nullptr)
        {
            args.Emplace("certificate", certificate);
        }
        
        String response(Allocator);
        response.Reserve(1024);
        
        SendBotRequest("setWebhook", args, response);
    }
    
    bool TelegramApi::SendBotRequest(String method, const TArray<Net::Http::HttpRequestParam>& parameters, String& response)
    {
        String stringUrl(TelegramBotApiEndpoint, Allocator);
        stringUrl += this->BotToken;
        stringUrl += "/";
        stringUrl += method;
        
        Net::Http::Url url(stringUrl);
        
        return SendRequest(url, parameters, response);
    }
}
