//抄袭没妈，倒卖没爹

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SparkApi.h>
#include <LED.h>

String APPID="";
String APIKey="";
String APISecret="";
String Spark_url="wss://spark-api.xf-yun.com/v3.5/chat";
String host="spark-api.xf-yun.com";
String path="/v3.5/chat";
String ssid="";
String password="";

String res = "";
Ws_Param* ws=nullptr;

void setup() {
  Serial.begin(115200);
  Serial.println("抄袭没妈，倒卖没爹");

  // WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // init 
  LED_init();
  ws=new Ws_Param(APPID,APIKey,APISecret,Spark_url,host,path);
}

void loop()
{
  // Enter my words
  Serial.print("yuanyuan: ");
  char add=0;
  while (add!='\n')
  {
    if(Serial.available()){
      add = Serial.read();
      res = res + add;
      delay(1);
    }
  }
  int len = res.length();
  res = res.substring(0, (len - 2));  //去除最后的13 10
  Serial.println(res);

  //这里写websocket发送和接收的逻辑
  message_add(res);
  ws->send_message(res);
  ws->process_message();
  String mes=Ws_Param::get_messagebuffer();
  //debug
  Serial.println(mes);
  Serial.print("assist:");
  Serial.println(action(mes));

  res = "";
  delay(1000);
}

