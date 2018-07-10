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
//  TelegramApi.cpp
//  Neko Framework
//
//  Created by Neko on 6/29/18.
//

#include "../../Engine/Network/Http/Url.h"
#include "../../Engine/Network/NetSocket.h"
#include "../../Engine/Containers/HashMap.h"
#include "../../Engine/Core/Log.h"

#include "TelegramApi.h"

#include "../SocketSSL.h"
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
    
    void TelegramApi::SendMessage(long chatId, String message, const char* parseMode/* = ""*/)
    {
        TArray<Net::Http::HttpRequestParam> args(Allocator);
        
        args.Reserve(3);
        args.Emplace("chat_id", (int64)chatId);
        args.Emplace("text", *message);
		args.Emplace("parse_mode", parseMode);
        
        String response(Allocator);
        response.Resize(1024);
        
        SendBotRequest("sendMessage", args, response);
    }
        
    void TelegramApi::SendSticker(long chatId, String id)
    {
        TArray<Net::Http::HttpRequestParam> args(Allocator);
        
        args.Reserve(2);
        args.Emplace("chat_id", (int64)chatId);
        args.Emplace("sticker", *id);
        
        String response(Allocator);
        response.Resize(1024);
        
        SendBotRequest("sendSticker", args, response);
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
		TArray<Net::Http::HttpRequestParam> args(Allocator);
        
        args.Reserve(2);
        args.Emplace("chat_id", (int64)chatId);
		
		const auto mimeType = Http::GetMimeByFileName(filename, GetPhotoMimeTypes());
		
        Net::Http::HttpRequestParam photoItem("photo", "", true, mimeType, filename);
        photoItem.value.Append((const char* )data, size);
        
        args.Emplace(photoItem);
		
        String response(Allocator);
        response.Resize(1024);
        
        SendBotRequest("sendPhoto", args, response);
	}
	
    bool TelegramApi::SendBotRequest(String method, const TArray<Net::Http::HttpRequestParam>& parameters, String& response)
    {
        String stringUrl(TelegramBotApiEndpoint, Allocator);
        stringUrl += this->BotToken.data;
        stringUrl += "/";
        stringUrl += method;
        
        Net::Http::Url url(stringUrl);
        
        return SendRequest(url, parameters, response);
    }
}
