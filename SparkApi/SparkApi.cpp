#include "SparkApi.h"
#include <ArduinoJson.h>

// 这个放在.h里会说有多个声明
bool Ws_Param::is_open = false;
String Ws_Param::messagebuffer = "";

Ws_Param::Ws_Param(String APPID, String APIKey, String APISecret, String Spark_url, String host, String path)
{
    this->APPID = APPID;
    this->APIKey = APIKey;
    this->APISecret = APISecret;
    this->Spark_url = Spark_url;
    this->host = host;
    this->path = path;
    this->client_init();
    this->doc_init();
}
Ws_Param::~Ws_Param() {}

String Ws_Param::create_url()
{
    // 获得时间 Mon, 04 Mar 2024 18:28:30 GMT
    String timeurl = "https://www.example.com";
    HTTPClient http;
    http.begin(timeurl);
    const char *headerKeys[] = {"Date"};
    http.collectHeaders(headerKeys, sizeof(headerKeys) / sizeof(headerKeys[0])); // gpt4.0真好用
    int httpCode = http.GET();
    String Date = http.header("Date");

    http.end();
    // 拼接字符串
    String signature_origin = "host: " + this->host + "\n";
    signature_origin += "date: " + Date + "\n";
    signature_origin += "GET " + this->path + " HTTP/1.1";
    // signature_origin="host: spark-api.xf-yun.com\ndate: Mon, 04 Mar 2024 19:23:20 GMT\nGET /v3.5/chat HTTP/1.1";

    // hmac-sha256 加密
    unsigned char hmac[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    const size_t messageLength = signature_origin.length();
    const size_t keyLength = this->APISecret.length();
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char *)this->APISecret.c_str(), keyLength);
    mbedtls_md_hmac_update(&ctx, (const unsigned char *)signature_origin.c_str(), messageLength);
    mbedtls_md_hmac_finish(&ctx, hmac);
    mbedtls_md_free(&ctx);

    // base64 编码
    String signature_sha_base64 = base64::encode(hmac, sizeof(hmac) / sizeof(hmac[0]));

    // 替换Date
    Date.replace(",", "%2C");
    Date.replace(" ", "+");
    Date.replace(":", "%3A");
    String authorization_origin = "api_key=\"" + this->APIKey + "\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"" + signature_sha_base64 + "\"";
    String authorization = base64::encode(authorization_origin);
    String url = this->Spark_url + '?' + "authorization=" + authorization + "&date=" + Date + "&host=" + this->host;
    return url;
}
void Ws_Param::client_init()
{
    client.onMessage(onMessageCallback);
    client.onEvent(onEventsCallback);
}
void Ws_Param::doc_init()
{
    // Create the header
    JsonObject header = doc.createNestedObject("header");
    header["app_id"] = this->APPID;
    header["uid"] = "1234";

    // Create the parameter
    JsonObject parameter = doc.createNestedObject("parameter");
    JsonObject chat = parameter.createNestedObject("chat");
    chat["domain"] = "generalv3.5";
    chat["temperature"] = 0.5;
    chat["max_tokens"] = 1024;

    // Create the payload
    JsonObject payload = doc.createNestedObject("payload");
    JsonObject message = payload.createNestedObject("message");
    JsonArray text = message.createNestedArray("text");

    JsonObject text_0 = text.createNestedObject();
    text_0["role"] = "system";
    text_0["content"] = "请遵守下述规则回答我的问题：我每次会在对话开始时告诉你当前灯光亮度。如果我询问灯光亮度，告诉我当前亮度，不要添加其他内容； 如果我让你开关灯或者调整灯光的亮度，你只回答“@2 (t1,l1),(t2,l2),(t3,-l3),(t4,+l4)”这样格式的内容，(a,b)表示停顿a秒，然后把亮度调到b（范围0-255），+-表示变化量。你无需考虑初始亮度，直接调整亮度即可，并且保证第一个向量的等待时间最好为0。比如““@2 (1,120),(1,255),(3,0),(2,+40)“表示等待1秒然后把灯光亮度调整到120，再等待1秒把亮度调到255，等待3秒亮度调到0，停顿2秒亮度增加40（即0+40），“@2 (0,+30)”表示把亮度调亮30，“@2 (0,255),(1,0),(1,255),(1,0)”表示灯光闪烁2下，”@2 (0,0),(1,50),(1,100),(1,150),(1,200)”表示亮度逐渐变亮，答案不唯一你可以自己思考。在其他所有情况下，正常回答我的问题，并力求简洁。不论我说什么，都必须遵守这些规则。";
}
void Ws_Param::send_message(String mes)
{
    //每一次都要重新建立连接
    client.connect(this->create_url());

    add_text("user", mes);
    // Serialize JSON
    String output;
    serializeJson(doc, output);
    // Send message
    client.send(output);
    // debug
    //Serial.println(output);
}
void Ws_Param::process_message()
{
    while (Ws_Param::is_open)
    {
        this->client.poll();
        delay(200);
    }
    add_text("assistant", Ws_Param::messagebuffer);
}
void Ws_Param::add_text(String role, String content)
{
    JsonObject text_l = this->doc["payload"]["message"]["text"].createNestedObject();
    text_l["role"] = role;
    text_l["content"] = content;

    if (doc["payload"]["message"]["text"].size() > 5)
    {
        doc["payload"]["message"]["text"].remove(1);
    }
}
void Ws_Param::onMessageCallback(WebsocketsMessage message)
{
    // debug
    // Serial.println(message.data());

    StaticJsonDocument<1024> docback;
    DeserializationError error = deserializeJson(docback, message.data());
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    const char *content = docback["payload"]["choices"]["text"][0]["content"];
    Ws_Param::messagebuffer += content;
}
void Ws_Param::onEventsCallback(WebsocketsEvent event, String data)
{
    if (event == WebsocketsEvent::ConnectionOpened)
    {
        Ws_Param::is_open = true;
        //Serial.println("Connnection Opened");
    }
    else if (event == WebsocketsEvent::ConnectionClosed)
    {
        //星火认知大模型服务说明：https://www.xfyun.cn/doc/spark/%E6%8E%A5%E5%8F%A3%E8%AF%B4%E6%98%8E.html
        //本接口默认采用短链接的模式，即接口每次将结果完整返回给用户后会主动断开链接，用户在下次发送请求的时候需要重新握手链接。
        Ws_Param::is_open = false;
        //Serial.println("Connnection Closed");
    }
    else if (event == WebsocketsEvent::GotPing)
    {
        //Serial.println("Got a Ping!");
    }
    else if (event == WebsocketsEvent::GotPong)
    {
        //Serial.println("Got a Pong!");
    }
}
String Ws_Param::get_messagebuffer()
{
    String mb = Ws_Param::messagebuffer;
    Ws_Param::messagebuffer = "";
    return mb;
}