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
//  TelegramApi.cpp
//  Neko Framework
//
//  Created by Neko on 6/29/18.
//

#include "Engine/Network/Http/Url.h"
#include "Engine/Network/NetSocketBase.h"
#include "Engine/Containers/HashMap.h"
#include "Engine/Core/Log.h"

#include "TelegramApi.h"

#include "../Skylar/ISsl.h"
#include "../Sockets/SocketSSL.h"
#include "../Utils.h"

namespace Neko
{
    static const char* TelegramBotApiEndpoint = "https://api.telegram.org/bot";
    static const uint16 TelegramApiPort = 443;
    
    TelegramApi::TelegramApi(IAllocator& allocator)
    : Allocator(allocator)
    {
        // client context
# if USE_OPENSSL
        this->SslContext = SSL_CTX_new (::SSLv23_client_method());
# endif
    }
    
    TelegramApi::~TelegramApi()
    {
        assert(this->SslContext != nullptr);
        
# if USE_OPENSSL
        SSL_CTX_free((SSL_CTX* )this->SslContext);
# endif
    }
  
    void TelegramApi::SetBotToken(const char* token)
    {
        this->BotToken = token;
    }
    
    void TelegramApi::SetWebHook(const char* domain, const char* certificate/* = nullptr*/)
    {
        TArray<Http::HttpRequestParam> params(Allocator);
        
        params.Reserve(2);
        params.Emplace("url", domain);
        
        if (certificate != nullptr)
        {
            params.Emplace("certificate", certificate);
        }
        
        String response(Allocator);
        response.Reserve(1024);
        
        SendBotRequest("setWebhook", params, response);
    }
    
    bool TelegramApi::SendRequest(const Http::Url& url, const TArray<Http::HttpRequestParam>& parameters, String& response)
    {
        LogInfo.log("Telegram") << "Sending request " << *url.Host;
        
        Net::NetSocketBase socket;
        Net::Endpoint address;
        
        address = socket.Init(*url.Host, TelegramApiPort, Net::SocketType::Tcp);
        
        socket.MakeNonBlocking(false);
        socket.SetSocketStreamNoDelay();
        
        // connect using inner socket
        int32 result = socket.Connect(address);
        
        if (result < 0)
        {
            LogError.log("Telegram") << "SendRequest failed to connect";
            return false;
        }
        
        Skylar::SocketSSL socketSsl(socket, reinterpret_cast<SSL_CTX& >(SslContext));
        
        // connect using ssl
        result = socketSsl.Connect();
        if (result < 0)
        {
            LogError.log("Telegram") << "SendRequest failed to connect via SSL";
            return false;
        }
        
        const auto requestText = Http::Client::GenerateRequest(url, parameters, false);
        
        long bytes = socketSsl.SendAllPacketsWait(*requestText, requestText.Length(), -1);
        if (bytes < 0)
        {
            LogError.log("Telegram") << "Couldn't send request";
            return false;
        }
        
        bytes = socketSsl.GetPacketBlocking((void* )response.c_str(), response.Length(), -1);

        if (bytes <= 0)
        {
            LogWarning.log("Telegram") << "SendRequest couldn't get response back, hmm";
            return true;
        }
        
        socketSsl.Close();
        
        return true;
    }
    
    void TelegramApi::SendMessage(long chatId, String message, const char* parseMode/* = ""*/)
    {
        TArray<Http::HttpRequestParam> params(Allocator);
        
        params.Reserve(3);
        params.Emplace("chat_id", (int64)chatId);
        params.Emplace("text", *message);
		params.Emplace("parse_mode", parseMode);
        
        String response(Allocator);
        response.Resize(1024);
        
        SendBotRequest("sendMessage", params, response);
    }
        
    void TelegramApi::SendSticker(long chatId, String id)
    {
        TArray<Http::HttpRequestParam> params(Allocator);
        
        params.Reserve(2);
        params.Emplace("chat_id", (int64)chatId);
        params.Emplace("sticker", *id);
        
        String response(Allocator);
        response.Resize(1024);
        
        SendBotRequest("sendSticker", params, response);
    }
	
	// temp, get these from shared settings?
	static THashMap<String, String>& GetPhotoMimeTypes()
	{
		static DefaultAllocator allocator;
		static THashMap<String, String> mimeTypes(allocator);
		if (mimeTypes.GetSize() == 0)
		{
			mimeTypes.Insert("gif", "image/gif");
			mimeTypes.Insert("jpg", "image/jpeg");
			mimeTypes.Insert("jpeg", "image/jpeg");
			mimeTypes.Insert("png", "image/png");
		}
		return mimeTypes;
	}
	
	void TelegramApi::SendPhoto(long chatId, const char* filename, uint8* data, ulong size)
	{ 
		TArray<Http::HttpRequestParam> params(Allocator);
        
        params.Reserve(2);
        params.Emplace("chat_id", (int64)chatId);
		
		const auto mimeType = Skylar::GetMimeByFileName(filename, GetPhotoMimeTypes());
		
        Http::HttpRequestParam photoItem("photo", "", true, mimeType, filename);
        photoItem.value.Append((const char* )data, size);
        
        params.Emplace(photoItem);
		
        String response(Allocator);
        response.Resize(1024);
        
        SendBotRequest("sendPhoto", params, response);
	}
	
    bool TelegramApi::SendBotRequest(String method, const TArray<Http::HttpRequestParam>& parameters, String& response)
    {
        if (this->BotToken.IsEmpty())
        {
            return false;
        }
        
        String stringUrl(TelegramBotApiEndpoint, Allocator);
        stringUrl += this->BotToken.data;
        stringUrl += "/";
        stringUrl += method;
        
        Http::Url url(stringUrl);
        
        return SendRequest(url, parameters, response);
    }
}
