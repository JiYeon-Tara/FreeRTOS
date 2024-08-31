#include "sys.h"
#include "usart.h"		
#include "delay.h"	

#include "key.h"  

int main(void)
{
 	Stm32_Clock_Init(9);	//系统时钟设置
	uart_init(72, 2000000);	 	//串口初始化为9600
	delay_init(72);	   	 	//延时初始化 
	// LED_Init();		  		//初始化与LED连接的硬件接口
	// LCD_Init();			   	//初始化LCD 	
	// KEY_Init();				//按键初始化		 	
	// tp_dev.init();			//触摸屏初始化
 	// POINT_COLOR=RED;//设置字体为红色 
	// LCD_ShowString(60,50,200,16,16,"Mini STM32");	
	// LCD_ShowString(60,70,200,16,16,"TOUCH TEST");	
	// LCD_ShowString(60,90,200,16,16,"ATOM@ALIENTEK");
	// LCD_ShowString(60,110,200,16,16,"2014/3/11");
	printf("\r\n\r\n\r\n\r\n\r\n\r\n");
    printf("***************************************\r\n");
    printf("* SRAM APP start\n");
    printf("***************************************\n\n");

   	// if(tp_dev.touchtype!=0XFF)
	// 	LCD_ShowString(60,130,200,16,16,"Press KEY0 to Adjust");//电阻屏才显示
	// delay_ms(1500);
	// Load_Drow_Dialog();	 	
	// if(tp_dev.touchtype&0X80)ctp_test();	//电容屏测试
	// else rtp_test(); 						//电阻屏测试
	while (1) {
		printf("SRAM APP\r\n");
		delay_ms(10 * 1000);
	}
}



