#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include "board_config.h"
#include "bsp_config.h"
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h" 
#include "key_stm.h" 
#include "service_usart.h"
#include "watch_dog_timer.h"
#include "timer.h"
#include "LCD.h"
#include "ssd_1306.h"
#include "usmart.h"
#include "exti.h"
#include "third_party_config.h"
#include "at_cmd_parse.h"

 
// Program Size: Code=14966 RO-data=7166 RW-data=408 ZI-data=1848  
// FromELF: creating hex file...
// "..\OBJ\test.axf" - 0 Error(s), 13 Warning(s).
// Build Time Elapsed:  00:00:01
/**
 * @brief 编译信息解释, 代码占用 flash:14966 + 7166 = 22132 bytes
 * 占用 SRAM 大小:1848 + 408 = 1856 bytes
 * 
 * code 表示程序占用 flash 大小
 * RO-data 表示程序定义的常量(flash)
 * RW-data 表示程序定义的已经初始化的变量
 * ZI-data 表示未被初始化的变量
 * 
 * startup_stm32f10x_hd.s 中, 定义了堆栈(heap + stack) 总大小 0x600 = 1536 bytes, 
 * 
 */


void bsp_init()
{
	uint8_t lcd_id[20];

	Stm32_Clock_Init(9); 		 //系统时钟设置
	delay_init(72000000);	     //延时初始化
	uart_init(72000000, 9600);	 //串口初始化为9600
	LED_Init();					 // LED Init
	EXTI_Init(); 				 // key and exit initialization
	

	LCD_Init();
	sprintf((char*)lcd_id, "LCD ID:%04X", lcddev.id);
	LCD_ShowString(0, 0, 240, 16, 16, lcd_id);
	
}

int fun(void)
{
	// LCD_Init(); 		//感觉这个板子跑个LCD屏已经有点心有余而力不足了, 可以优化吗?
	// usmart_dev.init(72); //初始化 USMART
	
	// //IWDG_Init(4, 615); // watch dog intialization
	// //WWDG_Init(0X7F,0X5F,3);	//计数器值为7f,窗口寄存器为5f,分频数为8	 
	// TIM3_Int_Init(9999, 7199);	//10Khz的计数频率，计数到5000为500ms  

	while(1){
		//open watch dog
		// uart1_print_recv_msg(S_USART1);
		// LCD_Test(&ix);
		// printf("%d\n", KEY_Scan(1));
		// if(cmd == "circle loop"){
		// 	while(1);	//dog will bark
		// }

		//feed watch dog
		
		//printf("thread running...\n");
		//delay_ms(1000);
#if LED_ENABLE

#endif
	}
}

void led_test()
{
	LED_Init();		  	 //初始化与LED连接的硬件接口
	LED_Reset();
	delay_ms(5000);
	LED_Set();
	delay_ms(5000);
}

void key_test()
{
#if LOOP_KEY_ENABLE	
	uint8_t val;

	LED_Init();
	KEY_Init();
	uart_init(72000000, 9600);

	while(1){
		val = KEY_Scan(SINGLE_SHOT_MODE);
		// printf("val:%d\n", val);
		switch(val){
			case KEY0_PRES:
			{
				printf("val:%d\n", val);
				LED_Reset();
				break;
			}
			case KEY1_PRES:
			{
				printf("val:%d\n", val);
				LED_Set();
				break;
			}
			case WKUP_PRES:
			{
				printf("val:%d\n", val);
				LED_Reset();
				delay_ms(5000);
				LED_Set();
				delay_ms(5000);
				break;
			}
			default:
				break;
		}
	}
#else if INT_KEY_ENABLE
	EXTI_Init();
#endif
}

void usart_test()
{
#if USART1_ENABLE
	uint8_t len;
	uint8_t ix = 0;

	uart_init(72000000, 9600);	 //串口初始化为9600
	LED_Init();
	KEY_Init();

	while(1){
		if(USART_RX_STA & UART_RX_COMPLETE){
			len = USART_RX_STA & UART_GET_RX_LEN;
			printf("recv len:%d\n", len);
			for(ix = 0; ix < len; ++ix){
				printf("%c", USART_RX_BUF[ix]);
			}
			printf("\r\n");
			USART_RX_STA = 0;
		}
	}
#endif
}

void watch_dog_test()
{
#if IWATCH_DOG_TEST_ENABLE
	IWDG_Init(4, 625); // 溢出时间为 1s
#else if WWATCH_DOG_TEST_ENABLE
	
#endif
}

void oled_screen_test()
{

}

void lcd_screen_test()
{
	uint8_t lcd_id[20];

	LCD_Init();
	printf("LCD_Init\n");
	sprintf((char*)lcd_id, "LCD ID:%04X", lcddev.id);

	LCD_ShowString(0, 0, 240, 16, 16, lcd_id);
	LCD_Draw_Circle(20, 20, 10);
	LCD_DrawLine(40, 40, 80, 80);	
}

void at_cmd_test()
{
	char *str = "AT^START"; // receive from uart

	at_cmd_parse(str);

	return;
}















