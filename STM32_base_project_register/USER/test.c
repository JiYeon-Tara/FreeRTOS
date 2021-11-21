#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h" 
#include "key.h" 

 
int main(void)
{			
	u8 t;
	u8 len;	
	u16 times=0; 

	Stm32_Clock_Init(9); //系统时钟设置
	delay_init(72);	     //延时初始化 
	uart_init(72,9600);	 //串口初始化为9600
	LED_Init();		  	 //初始化与LED连接的硬件接口
	// LED_Set();
	// LED_Reset();

	while(1)
	{
		/* 串口处理逻辑, 通过全局变量 USART_RX_STA 实现 */
		//接收状态
		//bit15，	接收完成标志
		//bit14，	接收到0x0d
		//bit13~0，	接收到的有效字节数目
		if(USART_RX_STA & 0x8000)	//bit[15], 接收完成
		{					   
			len = USART_RX_STA & 0x3fff;//得到此次接收到的数据长度
			printf("\r\n您发送的消息为:\r\n");
			//再将从串口收到的数据从串口发送出去
			for(t = 0; t < len; t++)
			{
				USART1->DR = USART_RX_BUF[t];
				while((USART1->SR & 0X40) == 0)		//bit[6]， 发送完成标志位
					;//等待发送结束
			}
			printf("\r\n\r\n");//插入换行
			USART_RX_STA = 0;
		}else
		{
			times++;
			if(times % 300 == 0)
				printf("请输入数据,以回车键结束\r\n");  
				//LED1 = !LED1;
			if(times % 30 == 0)
				//LED0 =! LED0;//闪烁LED,提示系统正在运行.
			delay_ms(10);   
		}
	}	

	return 0; 
}














