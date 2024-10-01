#ifndef __USART_H
#define __USART_H
#include "sys.h"
#include <stdio.h>
#include <stdbool.h>


#define UART_RX_COMPLETE        0x8000  // bit15，	接收完成标志
#define UART_RX_0X0D            0x4000  // bit14，	接收到0x0d
#define UART_GET_RX_LEN         0x3FFF  // bit13~0，	接收到的有效字节数目

#define USART_REC_LEN  			200  	// 定义最大接收字节数 200
#define EN_USART1_RX 			1		// 使能（1）/禁止（0）串口1接收


extern uint8_t  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern uint16_t USART_RX_STA; // 接收状态标志位


void uart1_init(u32 pclk2, u32 bound); // 如果想串口中断接收，请不要注释以下宏定义
void uart1_send(const u8 *p_data, u16 len);

#endif	   
















