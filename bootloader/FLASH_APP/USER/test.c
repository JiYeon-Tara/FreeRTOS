#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"   
#include "ILI93xx.h" 
#include "usmart.h" 
#include "rtc.h"



// 修改 flash_app 地址之后需要烧录 boot 才可以运行, 因为默认是从 0x08000000 开始运行的,
int main(void)
{	
	u8 t;
	int usart_recv_len;
	int idx;

  	Stm32_Clock_Init(9);
	uart_init(72, 2000000);
	delay_init(72);
	printf("\r\n\r\n\r\n\r\n\r\n\r\n");
    printf("***************************************\r\n");
    printf("*Flash App start\r\n");
    printf("***************************************\r\n");
	// LED_Init();
 	LCD_Init();
	usmart_dev.init(72);	 	
	POINT_COLOR=RED;
	LCD_ShowString(60,50,200,16,16,"Flash App");	


	while(RTC_Init()) {
		LCD_ShowString(60,130,200,16,16,"RTC ERROR!   ");	
		delay_ms(800);
		LCD_ShowString(60,130,200,16,16,"RTC Trying...");	
	}
	    
	while (1) {
		if (USART_RX_STA & 0XC0) { // 串口是否接收完成
			usart_recv_len = USART_RX_STA & 0X3F;
			printf("usart_recv_len:%d", usart_recv_len);
			for (idx = 0; idx < usart_recv_len; ++idx)
				printf("%X", USART_RX_BUF[idx]);
			printf("\r\n");
		}
		delay_ms(10);
	};  	
}


