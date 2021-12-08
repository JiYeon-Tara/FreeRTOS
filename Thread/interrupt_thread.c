#include "interrupt_thread.h"

#include "delay.h"
#include "led.h"
#include "thread_ui.h"

//
// task Handler
//
TaskHandle_t INTERRUPTTask_Handler;

/**
 * @brief interrupt task entry function
 * 
 * @param pvParameters 
 */
void interrupt_task(void *pvParameters)
{
	static u32 total_num=0;
	//resource_init()
	//hardware_init()
	//software_init()
	//other init
	
    while(1)
    {
		total_num += 1;
		//printf("\r\nconfigMAX_SYSCALL_INTERRUPT_PRIORITY = %d\r\n", configMAX_SYSCALL_INTERRUPT_PRIORITY);						

		if(total_num % 10 == 0) 
		{
			printf("close interrupt.............\r\n");
			LED0 = 0;
            //close interrupt
			//低于 configMAX_SYSCALL_INTERRUPT_PRIORITY 中断被屏蔽, 无法执行
			//关闭中断之后之后优先级大于 configMAX_SYSCALL_INTERRUPT_PRIORITY 的中断不会被屏蔽
			portDISABLE_INTERRUPTS();           
			delay_xms(5000);
			// printf("\r\nconfigMAX_SYSCALL_INTERRUPT_PRIORITY = %d\r\n", configMAX_SYSCALL_INTERRUPT_PRIORITY);						
			printf("open interrupt.............\r\n");
            //open interrupt

			portENABLE_INTERRUPTS();
			//删除 UI 线程
			//vTaskDelete(Thread_UI_Handler);
			printf("delete ui thread.\r\n");
		}
        LED0 = 1;
        vTaskDelay(1000);
    }
} 

