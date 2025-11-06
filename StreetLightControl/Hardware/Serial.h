#ifndef __SERIAL_H__
#define __SERIAL_H__ 

#include <stdio.h>

#define RX_BUFFER_SIZE 1024
#define USART_BUFF_SIZE 1024

// USART1 接收缓冲区
extern uint8_t usart1_rx_buffer[500];
extern unsigned int usart1_rx_index;
extern uint8_t usart1_rx_flag;

// USART2 接收缓冲区
extern char usart2_rx_buffer[500];
extern unsigned int usart2_rx_index;
extern uint8_t usart2_rx_flag;

void USART1_Init(void);
void USART2_Init(void);
void Serial_SendByte(USART_TypeDef* USARTx, uint8_t byte);
void Serial_SendString(USART_TypeDef* USARTx, uint8_t *str);
uint8_t Usart_ReceiveByte(USART_TypeDef* USARTx, uint8_t byte);
uint8_t Usart_Receive_str(USART_TypeDef* USARTx, uint8_t* str);
void my_printf(USART_TypeDef *USARTx, char *fmt, ...);

#endif
