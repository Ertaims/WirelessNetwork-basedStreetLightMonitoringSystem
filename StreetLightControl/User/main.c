#include "stm32f10x.h"  // Device header
#include "Delay.h"
#include "OLED.h"
//#include "PWM.h"
#include "esp8266-01s.h"


/*********************************************************************************
  * USART2作为ESP8266的通信接口，USART1作为调试接口
  * 硬件引脚连接：
  * ESP8266    STM32
  * VCC	 ->  3.3V
  * GND	 ->  GND
  * TXD	 ->  PA3 (USART2 RX)
  * RXD	 ->  PA2 (USART2 TX)
  * RST	 ->  PA4 (GPIO Output)
  * 
  * 调试串口连接：
  * CH340  STM32
  * GND	 ->  GND
  * TXD	 ->  PA10 (USART1 RX)
  * RXD	 ->  PA9  (USART1 TX)
*********************************************************************************/
int main(void)
{
    // PWM_Init();
	Delay_ms(1000);

	ESP8266_Init();
	
	while(ESP8266_ConnectMQTT())
    {
        my_printf(USART1, "Failed to connect to MQTT, retrying...\r\n");
        Delay_ms(5000); // 等待5秒后重试
    }

    Delay_ms(500);

    MQTT_Subscribe("lamp/control/+", 1);

    MQTT_Publish("lamp/control/lamp_001", "{\\\"cmd\\\":\\\"ON\\\"}", 1);
    //MQTT_Subscribe("esp8266/01s", 1);

    // for(int i = 0; i < 5; i++)
    // {
    //     MQTT_Publish("esp8266/01s", "{\"temperature\":25,\"humidity\":60}", 1);
    //     Delay_ms(2000);
    // }

	while(1)
	{
	}
}
