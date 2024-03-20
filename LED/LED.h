#ifndef _LED_H
#define _LED_H
#include <Arduino.h>
// 宏定义 GPIO 输出引脚
#define LED_PIN 19

extern int brightness; //类似于变量的声明

void LED_init();
void LED(int i);
void message_add(String& mes);
String action(String data);
#endif