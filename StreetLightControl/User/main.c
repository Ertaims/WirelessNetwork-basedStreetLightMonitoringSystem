#include "stm32f10x.h"  // Device header
#include "Delay.h"
#include "OLED.h"
//#include "PWM.h"
#include "esp8266-01s.h"


/** 
  * USART2作为ESP8266的通信接口，USART1作为调试接口
  */
int main(void)
{
	// OLED_Init();
	// PWM_Init();
	Delay_ms(1000);

	ESP8266_Init();
	
	while(ESP8266_ConnectMQTT())

	while(1)
	{
		
	}
}
