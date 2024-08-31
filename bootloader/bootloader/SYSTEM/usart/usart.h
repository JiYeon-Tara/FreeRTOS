#ifndef __USART_H
#define __USART_H
#include "sys.h"
#include "stdio.h"	 


// TODO:
// 由于目前测试的方案是, 一次性接收完成后, 然后再进行写 flash, 然后跳转
// 由于这里设置了串口最大接收 41KB
// 因此 flash app 最大只能接受 41KB, 这里可以优化方案
#define USART_REC_LEN  			41*1024 //定义最大接收字节数 41K
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
extern u16 USART_RX_CNT;				//接收的字节数
//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 pclk2,u32 bound);

#endif	   
















