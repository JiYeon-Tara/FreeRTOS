#ifndef __USART_H
#define __USART_H
#include "sys.h"
#include <stdio.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////////////
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
//V1.6修改说明 20150109
//uart_init函数去掉了开启PE中断
//V1.7修改说明 20150322
//修改OS_CRITICAL_METHOD宏判断为：SYSTEM_SUPPORT_OS
////////////////////////////////////////////////////////////////////////////////// 	  

// macro
#define UART_RX_COMPLETE        0x8000  // bit15，	接收完成标志
#define UART_RX_0X0D            0x4000  // bit14，	接收到0x0d
#define UART_GET_RX_LEN         0x3FFF  // bit13~0，	接收到的有效字节数目

#define USART_REC_LEN  			200  	// 定义最大接收字节数 200
#define EN_USART1_RX 			1		// 使能（1）/禁止（0）串口1接收

// global varialbes
extern uint8_t  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern uint16_t USART_RX_STA; // 接收状态标志位


// function
void uart_init(u32 pclk2, u32 bound);   // 如果想串口中断接收，请不要注释以下宏定义
bool at_cmd_parse(const char *str1, const char *str2);

#endif	   
















