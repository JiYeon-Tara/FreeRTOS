#include "test.h"
 
int main(void)
{			
	Stm32_Clock_Init(9); 	//系统时钟设置
	delay_init(72000000);	     //延时初始化 
	uart_init(72000000, 9600);	 //串口初始化为9600
	LED_Init();		  	 //初始化与LED连接的硬件接口
	KEY_Init();
	EXTI_Init();
	printf("intialized.");
	
	//IWDG_Init(4, 615); // watch dog intialization
	//WWDG_Init(0X7F,0X5F,3);	//计数器值为7f,窗口寄存器为5f,分频数为8	 
	TIM3_Int_Init(9999, 7199);	//10Khz的计数频率，计数到5000为500ms  

	while(1)
	{
		//open watch dog
		uart1_print_recv_msg(S_USART1);
		// printf("%d\n", KEY_Scan(1));
		// if(cmd == "circle loop"){
		// 	while(1);	//dog will bark
		// }

		//feed watch dog
		
		//printf("thread running...\n");
		//delay_ms(1000);
	}

	
}














