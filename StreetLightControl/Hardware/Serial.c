#include "stm32f10x.h"                  // Device header
#include "Serial.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// USART1 接收缓冲区
uint8_t usart1_rx_buffer[500];
unsigned int usart1_rx_index = 0;
uint8_t usart1_rx_flag = 0;

// USART2 接收缓冲区
char usart2_rx_buffer[500];
unsigned int usart2_rx_index = 0;
uint8_t usart2_rx_flag = 0;

/**
 * @brief  串口1初始化 - 连接esp8266
 * @param  无
 * @retval 无
 */
void USART1_Init(void)
{
    // 初始化RCC时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 初始化GPIO
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // USART1 TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // USART1 RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 串口初始化
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200; // 波特率115200
    USART_InitStructure.USART_HardwareFlowControl  = USART_HardwareFlowControl_None; // 无硬件流控
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 收发模式
    USART_InitStructure.USART_Parity = USART_Parity_No; // 无奇偶校验
    USART_InitStructure.USART_StopBits = USART_StopBits_1; // 1位停止位
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 8位数据位
    USART_Init(USART1, &USART_InitStructure);

    USART_ClearFlag(USART1, USART_FLAG_RXNE); // 清除中断标志位
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // 使能接收中断
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE); // 使能空闲中断

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组

    // 配置USART1中断
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; // USART1中断
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 使能USART1中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // 子优先级1
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART1, ENABLE); // 使能串口
}

/** 
 * @brief  串口2初始化 - 调试串口
 * @param  无
 * @retval 无
 */
void USART2_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    USART_DeInit(USART2); // 复位USART2

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; // USART2 TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; // USART2 RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200; // 波特率115200
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 收发模式
    USART_InitStructure.USART_Parity = USART_Parity_No; // 无奇偶校验
    USART_InitStructure.USART_StopBits = USART_StopBits_1; // 1位停止位
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 8位数据位
    USART_Init(USART2, &USART_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // 使能接收中断
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE); // 使能空闲中断

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组

    NVIC_InitTypeDef NVIC_InitStructure; 
    NVIC_InitStructure.NVIC_IRQChannel =  USART2_IRQn; // USART2中断
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 使能USART2中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; // 子优先级2
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART2, ENABLE);
}

/** 
 * @brief  发送一个字节
 * @param  USARTx 串口号，byte 要发送的字节
 * @retval 无
 */
void Serial_SendByte(USART_TypeDef* USARTx, uint8_t byte)
{
    USART_SendData(USARTx, byte);
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET); // 等待发送完成
}

/** 
 * @brief  发送字符串
 * @param  USARTx 串口号，str 要发送的字符串
 * @retval 无
 */
void Serial_SendString(USART_TypeDef* USARTx, uint8_t *str)
{
    while (*str)
    {
        Serial_SendByte(USARTx, *str);
        str++;
    }
}

/** 
 * @brief  接收一个字节
 * @param  USARTx 串口号
 * @retval 接收到的字节
 */
uint8_t Usart_ReceiveByte(USART_TypeDef* USARTx, uint8_t byte)
{
    while (USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET); // 等待接收完成
    return USART_ReceiveData(USARTx);
}

/** 
 * @brief  接收字符串
 * @param  USARTx 串口号，str 接收缓冲区
 * @retval 无
 */
uint8_t Usart_Receive_Buff[USART_BUFF_SIZE];
uint8_t Usart_Receive_str(USART_TypeDef* USARTx, uint8_t* str)
{
    int i = 0;
    if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE)) // USART_FLAG_RXNE置一时，接收数据寄存器有数据
    {
        Usart_Receive_Buff[i] = Usart_ReceiveByte(USARTx, *str++);
    }
    return 0;
}

/**
 * @brief 重定向c库函数printf到串口2
 * @param ch 传入的字符
 * @param f 文件指针
 * @retval 传回字符
 */
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (uint8_t)ch);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    return ch;
}

/** @brief 串口格式化输出 */
char Usart_buff[1024];
void my_printf(USART_TypeDef *USARTx, char *fmt, ...)
{
    va_list args; // 定义变参指针，用于获取参数列表中的参数
    va_start(args, fmt); // 初始化变参指针，指向第一个可变参数
    vsprintf((char *)Usart_buff, fmt, args);
    va_end(args);
    Serial_SendString(USARTx, (uint8_t *)Usart_buff);
    memset(Usart_buff, 0, sizeof(Usart_buff));
}

/** 
 * @brief  串口1中断服务程序
 * @param  无
 * @retval 无
 */
void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE))
    {
        Usart_Receive_Buff[usart1_rx_index++] = USART_ReceiveData(USART1);//保存接受到的数据
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
    if(USART_GetITStatus(USART1, USART_IT_IDLE))
    {
        USART1->SR;
        USART1->DR;
        usart1_rx_flag = 1;//表示接受完成
        usart1_rx_index = 0;

    }
}

/** 
 * @brief  串口2中断服务程序
 * @param  无
 * @retval 无
 */
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)   //如果USART_IT_RXNE标志置位，表示有数据到了，进入if分支
    {
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
        usart2_rx_buffer[usart2_rx_index] =  USART2->DR; //保存到缓冲区
        usart2_rx_index ++;                        //每接收1个字节的数据，Usart2_RxCounter加1，表示接收的数据总量+1			
    }
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        usart2_rx_flag = 1;//接收完成
        USART2->SR;                                                    //清除USART_IT_IDLE标志  步骤1
        USART2->DR;                                                    //清除USART_IT_IDLE标志  步骤2
		usart2_rx_index=0;
    }
}
