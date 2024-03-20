#include <LED.h>

int brightness = 0; // 在这里定义

void LED_init()
{
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0);
}

void LED(int i)
{
  i = min(255, max(0, i));
  analogWrite(LED_PIN, i);
  brightness = i;
}
void message_add(String& mes){
  mes="当前亮度为"+String(brightness)+"。"+mes;
}
String action(String data)
{
  if (data[0] == '@' && data[1] == '1')
  {
    // @1 备用
    return "";
  }
  else if (data[0] == '@' && data[1] == '2')
  {
    // @2 (255,1)
    int start = 0;
    int end = 3;
    while ((start = data.indexOf('(', end)) != -1 && (end = data.indexOf(')', start)) != -1)
    {
      String pair = data.substring(start + 1, end);
      int comma = pair.indexOf(',');

      String first = pair.substring(0, comma);
      String second = pair.substring(comma + 1);
      int action = 0;
      // Check if second number has '+' or '-' or none
      if (second[0] == '+')
      {
        //Serial.println("Second number has '+'");
        second = second.substring(1); // Remove '+'
        action = 1;
      }
      else if (second[0] == '-')
      {
        //Serial.println("Second number has '-'");
        second = second.substring(1);
        action = 2;
      }
      else
      {
        //Serial.println("Second number has no sign");
      }

      // Convert strings to integers
      float firstNumber = first.toFloat();
      int secondNumber = second.toInt();

      // Print numbers
      //Serial.print("First number: ");
      //Serial.println(firstNumber);
      //Serial.print("Second number: ");
      //Serial.println(secondNumber);

      switch (action)
      {
      case 0:
        delay((int)(firstNumber * 1000));
        LED(secondNumber);
        break;
      case 1:
        delay((int)(firstNumber * 1000));
        LED(brightness + secondNumber);
        break;
      case 2:
        delay((int)(firstNumber * 1000));
        LED(brightness - secondNumber);
        break;
      }
      // Move to next pair
      end++;
    }
    return "操作完毕";
  }
  return data;
}