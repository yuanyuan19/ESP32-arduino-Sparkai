#ifndef _SPARKAPI_H
#define _SPARKAPI_H

#include <HTTPClient.h>
	
#include <mbedtls/md.h>
#include <base64.h>

#include <ArduinoWebsockets.h>
#include <ArduinoJson.h> 

using namespace websockets;

class Ws_Param {
public:
    Ws_Param(String APPID, String APIKey, String APISecret, String Spark_url,String host,String path);
    ~Ws_Param();

    String create_url();
    void client_init();
    void doc_init();

    void send_message(String message);
    void process_message();
    void add_text(String role,String content);

    //喵d，回调函数得是静态的，所以is_open和messagebuffer也只能写成静态的了
    static bool is_open;
    static String messagebuffer;
    static void onMessageCallback(WebsocketsMessage message);
    static void onEventsCallback(WebsocketsEvent event, String data);
    static String get_messagebuffer();

    String APPID;
    String APIKey;
    String APISecret;
    String Spark_url;
    String host;
    String path;
    WebsocketsClient client;
    StaticJsonDocument<8192> doc;
};

#endif 
