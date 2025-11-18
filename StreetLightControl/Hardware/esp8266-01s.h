#ifndef __ESP8266_01S_H__
#define __ESP8266_01S_H__ 

#include "Serial.h"
#include "delay.h"
#include <string.h>

#define ESP_Receive_Buff 	usart2_rx_buffer  
#define ESP8266_USART 		USART2_Init  	
#define ESP8266_Rx_OK 		usart2_rx_flag  

#define MQTT_BROKER_IP "10.201.40.164"  // MQTT服务器IP地址
#define MQTT_BROKER_PORT "1883"
#define MQTT_BROKER_CLIENT_ID "ESP8266_01S"  // MQTT客户端ID
#define MQTT_BROKER_USER "cxk"
#define MQTT_BROKER_PASSWORD "123456"
#define MQTT_BROKER_TOPIC "esp8266/01s"
#define MQTT_BROKER_QOS 0

#define WIFI_SSID "OnePlusAce3"  // WiFi名称
#define WIFI_PASSWORD "a53sngf6"  // WiFi密码

#define RESET_IO(x)    GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)x)  //PA4控制WiFi的复位

// char mqtt_topic_buffer[];
// char mqtt_payload_buffer[];

void ESP8266_Init(void);
void ESP8266_RST_Init(void);
unsigned char ESP8266_SendCmd(char* cmd, int wait, char* ack);
char ESP8266_ConnectMQTT(void);
void MQTT_Subscribe(char* topic, uint8_t qos);
void MQTT_Publish(char* topic, char* payload, uint8_t qos);

#endif
