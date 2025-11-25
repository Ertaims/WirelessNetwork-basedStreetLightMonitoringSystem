#include "stm32f10x.h"                  // Device header
#include "esp8266-01s.h"

/**
 * @brief  初始化GPIO、串口等资源
 * @param  无
 * @retval 无
 */
void ESP8266_Init(void)
{
    //printf ( "\r\n正在初始化 ESP8266 ......\r\n" );
    USART1_Init();
    USART2_Init();
}

/**
 * @brief 初始化WiFi复位引脚
 * @param 无
 * @retval 无
 */
void ESP8266_RST_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB,PE端口时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //LED0-->PB.5 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

/**
 * @brief  发送命令
 * @param  cmd: 需要发送的指令
 * @param  wait: 等待时间
 * @param  ack: 期望的回复
 * @retval 0: 成功，1: 失败
 */
unsigned char ESP8266_SendCmd(char* cmd, int wait, char* ack)
{
    Delay_ms(10);
    memset(ESP_Receive_Buff, 0, sizeof(ESP_Receive_Buff));       //清空WiFi接收缓冲区
    my_printf(USART2, "%s\r\n", cmd);                     //发送AT指令到esp8266
    my_printf(USART1, "ESP8266 CMD: %s\r\n", cmd);
    
    while(wait--)
    {
        if(strstr((char *)ESP_Receive_Buff, (char *)ack) != NULL)
        {
            break;
        }
        Delay_ms(500);
        //my_printf(USART1, "ESP8266 ACK: %s\r\n", ESP_Receive_Buff);
    }
    my_printf(USART1, "\r\n");//打印一个换行
    if(wait <= 0) return 1; //失败
    else return 0; //成功
}

/**
 * @brief  发送数据
 * @param  data: 数据
 * @retval 无
 */

char ESP8266_ConnectMQTT(void)
{
    my_printf(USART1, "正在初始化ESP8266......\r\n");
    // 先对ESP8266模块进行硬件复位
    my_printf(USART1, "正在复位ESP8266......\r\n");
    ESP8266_RST_Init();
    RESET_IO(0); // 复位引脚拉低电平
    Delay_ms(1000);
    RESET_IO(1); // 复位引脚拉高电平

    ESP8266_SendCmd("AT+RST\r\n", 5, "ready"); // 进行软件复位

    my_printf(USART1, "复位完成，进行设置上电不自动连接路由器\r\n");

    if((ESP8266_SendCmd("AT+CWAUTOCONN=0\r\n", 10, "OK")))
    {
        my_printf(USART1, "WIFI上电不自动连接热点失败\r\n");
        return 10;
    }
	my_printf(USART1, "设置成功，进行设置模式\r\n");
		
	ESP8266_SendCmd("ATE0\r\n", 1, "OK");//关闭回显

    if((ESP8266_SendCmd("AT+CWMODE=1\r\n", 15, "OK"))) //设置为STA模式
    {
        my_printf(USART1, "wifi模式设置失败\r\n");
        return 1;
    }
	my_printf(USART1, "模式设置成功，准备连接路由器\r\n");

    if((ESP8266_SendCmd("AT+CWJAP=\""WIFI_SSID"\",\""WIFI_PASSWORD"\"\r\n", 25, "OK")))
    {
        my_printf(USART1, "路由器连接失败\r\n");
        return 2;
    }
    my_printf(USART1, "路由器连接成功,准备连接服务器\r\n");

    // 配置MQTT用户鉴权信息
    if((ESP8266_SendCmd("AT+MQTTUSERCFG=0,1,\""MQTT_BROKER_CLIENT_ID"\",\""MQTT_BROKER_USER"\",\""MQTT_BROKER_PASSWORD"\",0,0,\"\"\r\n", 25, "OK")))
    {
        my_printf(USART1, "MQTT用户鉴权信息配置失败\r\n");
        return 3;
    }
    my_printf(USART1, "MQTT用户鉴权信息配置成功,准备连接MQTT服务器\r\n");

    // 连接到MQTT服务器
    if((ESP8266_SendCmd("AT+MQTTCONN=0,\"10.201.40.164\",1883,1\r\n", 50, "OK")))
    {
        my_printf(USART1, "MQTT服务器连接失败\r\n");
        return 4;
    }
    my_printf(USART1, "MQTT服务器连接成功\r\n");

    return 0;
}

/**
 * @brief  订阅Topic
 * @param  topic: 要订阅的Topic
 * @param  qos: QoS等级
 * @retval 无
 */
void MQTT_Subscribe(char* topic, uint8_t qos)
{
    char cmd[200];
    my_printf(USART1, "准备订阅Topic: %s\r\n", topic);
    sprintf(cmd, "AT+MQTTSUB=0,\"%s\",%d\r\n", topic, qos);

    if((ESP8266_SendCmd(cmd, 30, "OK")))
    {
        my_printf(USART1, "MQTT订阅失败\r\n");
        return ;
    }
    my_printf(USART1, "MQTT订阅成功\r\n");
}

/**
 * @brief  发布Topic
 * @param  topic: 要发布的Topic
 * @param  payload: 发布内容
 * @param  qos: QoS等级
 * @retval 无
 */
void MQTT_Publish(char* topic, char* payload, uint8_t qos)
{
    char cmd[200];
    sprintf(cmd, "AT+MQTTPUB=0,\"%s\",\"%s\",%d,0\r\n", topic, payload, qos);
    my_printf(USART1, "准备发布Topic: %s\r\n", topic);
    my_printf(USART1, "Topic内容: %s\r\n", payload);

    if((ESP8266_SendCmd(cmd, 30, "OK")))
    {
        my_printf(USART1, "MQTT发布失败\r\n");
        return ;
    }
    my_printf(USART1, "MQTT发布成功\r\n");
}

// 全局变量用于存储接收到的消息
char mqtt_topic_buffer[128];
char mqtt_payload_buffer[2048];

// MQTT消息回调函数指针
void (*MQTT_MessageCallback)(char* topic, char* payload) = NULL;

/**
 * @brief 处理接收到的MQTT消息
 * @param 无
 * @retval 无
 */
void ESP8266_ProcessReceivedMessage(void)
{
    char* response_str = (char*)usart2_rx_buffer;

    // 添加长度检查
    if(strlen(response_str) > sizeof(usart2_rx_buffer))
    {
        my_printf(USART1, "接收到的消息长度过长，请检查!\r\n");
        return;
    }

    my_printf(USART1, "接收到MQTT消息:\r\n%s\r\n", response_str);
    // 查找MQTT接收消息的标识 "+MQTTSUBRECV"
    if (strstr(response_str, "+MQTTSUBRECV:") != NULL)
    {
        // 解析消息格式: +MQTTSUBRECV:0,"topic",payload_length,payload
        int payload_length;

        // 提取参数
        if(sscanf(response_str, "+MQTTSUBRECV:%*d,\"%[^\"]\",%d,", mqtt_topic_buffer, &payload_length) == 2)
        {
            // 定位JSON内容起始位置（即第一个'{'字符）
            char* json_start = strchr(strstr(response_str, "\",") + 2, '{');

            my_printf(USART1, "完整JSON内容: %s\n", json_start);

            my_printf(USART1, "接收到MQTT消息:\r\n");
            my_printf(USART1, "Topic: %s\r\n", mqtt_topic_buffer);
            my_printf(USART1, "Payload: %s\r\n", json_start);

            // 调用回调函数处理消息
            if (MQTT_MessageCallback != NULL)
            {
                MQTT_MessageCallback(mqtt_topic_buffer, json_start);
            }
        }
    }
    memset(usart2_rx_buffer, 0, sizeof(usart2_rx_buffer));
}

/**
 * @brief 设置MQTT消息接收回调函数
 * @param callback: 回调函数指针
 * @retval 无
 */
void ESP8266_SetMessageCallback(void (*callback)(char* topic, char* payload)) {
    MQTT_MessageCallback = callback;
}

/**
 * @brief 根据话题解析命令
 * @param payload: 控制命令
 * @retval 无
 */
void handle_mqtt_message(char* topic, char* payload)
{
    if (strstr(topic, "lamp/control/") != NULL)
    {
        parse_control_command(payload);
    }
    else if (strstr(topic, "lamp/status/") != NULL)
    {
        parse_status_command(payload);
    }
    else
    {
        my_printf(USART1, "未知的Topic: %s\r\n", topic);
    }
}
