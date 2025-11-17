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

    Delay_ms(500);

    NQTT_Subscribe("lamp/control/+", 1);
    NQTT_Publish("lamp/control/lamp_001", "{\"cmd\":\"ON\"}", 1);

	while(1)
	{
		
	}
}
