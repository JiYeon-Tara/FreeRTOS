#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"   
#include "lcd.h" 
#include "usmart.h" 
#include "rtc.h" 	 	 



// 修改 flash_app 地址之后需要烧录 boot 才可以运行, 因为默认是从 0x08000000 开始运行的,
int main(void)
{	
	u8 t;		   
  	Stm32_Clock_Init(9);	//ϵͳʱ������
	uart_init(72, 115200);	 	//���ڳ�ʼ��Ϊ9600
	delay_init(72);	   	 	//��ʱ��ʼ�� 
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
 	LCD_Init();			   	//��ʼ��LCD
	usmart_dev.init(72);	//��ʼ��USMART				 	
	POINT_COLOR=RED;//��������Ϊ��ɫ 
	LCD_ShowString(60,50,200,16,16,"Mini STM32");	
	LCD_ShowString(60,70,200,16,16,"RTC TEST");	
	LCD_ShowString(60,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(60,110,200,16,16,"2014/3/15");	

	// 0x08008004	
	// printf("Reset_Handler address:%p\r\n", Reset_Handler); // 怎么打印汇编函数的地址
	printf("flash app start\r\n\r\n\r\n\r\n\r\n\r\n");  

	while(RTC_Init())		//RTC��ʼ��	��һ��Ҫ��ʼ���ɹ�
	{ 
		LCD_ShowString(60,130,200,16,16,"RTC ERROR!   ");	
		delay_ms(800);
		LCD_ShowString(60,130,200,16,16,"RTC Trying...");	
	}		    						
	//��ʾʱ��
	POINT_COLOR=BLUE;//��������Ϊ��ɫ					 
	LCD_ShowString(60,130,200,16,16,"    -  -     ");	   
	LCD_ShowString(60,162,200,16,16,"  :  :  ");	 		    
	while(1)
	{								    
		if(t!=calendar.sec)
		{
			t=calendar.sec;
			LCD_ShowNum(60,130,calendar.w_year,4,16);									  
			LCD_ShowNum(100,130,calendar.w_month,2,16);									  
			LCD_ShowNum(124,130,calendar.w_date,2,16);	 
			switch(calendar.week)
			{
				case 0:
					LCD_ShowString(60,148,200,16,16,"Sunday   ");
					break;
				case 1:
					LCD_ShowString(60,148,200,16,16,"Monday   ");
					break;
				case 2:
					LCD_ShowString(60,148,200,16,16,"Tuesday  ");
					break;
				case 3:
					LCD_ShowString(60,148,200,16,16,"Wednesday");
					break;
				case 4:
					LCD_ShowString(60,148,200,16,16,"Thursday ");
					break;
				case 5:
					LCD_ShowString(60,148,200,16,16,"Friday   ");
					break;
				case 6:
					LCD_ShowString(60,148,200,16,16,"Saturday ");
					break;  
			}
			LCD_ShowNum(60,162,calendar.hour,2,16);									  
			LCD_ShowNum(84,162,calendar.min,2,16);									  
			LCD_ShowNum(108,162,calendar.sec,2,16);
			LED0=!LED0;
		}	
		delay_ms(10);								  
	};  	
}


