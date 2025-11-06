#include "stm32f10x.h"                  // Device header
#include "esp8266-01s.h"
#include "Serial.h"
#include "delay.h"
#include "string.h"

void ESP8266_Init(void)
{
    Serial_Init();
}

void ESP8266_SendCmd(char* cmd)
{
    Serial_SendString(cmd);
}

void ESP8266_SendData(char* data)
{
    char buffer[100];
    sprintf(buffer, "AT+CIPSEND=%s\r\n", strlen(data));
    ESP8266_SendCmd(buffer);
    delay_ms(100);
    ESP8266_SendCmd(data);
}

void ESP8266_Reset(void)
{
    ESP8266_SendCmd("AT+RST\r\n");
}

void ESP8266_AT_Test(void)
{
    ESP8266_SendCmd("AT\r\n");
}

/**
 * @brief  设置工作模式
 * @param  mode: 工作模式
 * @retval 无
 */
void ESP8266_SetMode(ENUM_Net_ModeTypeDef mode)
{
    switch ()
    {
        case STA:
            ESP8266_SendCmd("AT+CWMODE=1\r\n");
            break;
        case AP:
            ESP8266_SendCmd("AT+CWMODE=2\r\n");
            break;
        case STA_AP:
            ESP8266_SendCmd("AT+CWMODE=3\r\n");
            break;
    }
}


