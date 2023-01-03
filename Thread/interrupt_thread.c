#include "interrupt_thread.h"

#include "delay.h"
#include "led.h"
#include "thread_ui.h"

/********************
 * MACRO
 ********************/


/********************
 * FUNCTION
 ********************/
static void interrupt_task(void *pvParameters);
static void interrupt_task_exit(void *param);


/********************
 * GLOBAL VAR
 ********************/
thread_cb_t interrupt_thread = {
	.thread_init = interrupt_task,
	.thread_deinit = interrupt_task_exit,
};

// TaskHandle_t INTERRUPTTask_Handler;


static void resource_init()
{

}

static void hardware_init()
{

}

static void software_init()
{

}

/**
 * @brief interrupt task entry function
 * 
 * @param pvParameters 
 */
static void interrupt_task(void *pvParameters)
{
	static u32 total_num = 0;

	taskENTER_CRITICAL();
	hardware_init();
	resource_init();
	software_init();
	//other init
	printf("thread interrupt running...\r\n");
	taskEXIT_CRITICAL();
    while(1)
    {
		total_num += 1;
		//printf("\r\nconfigMAX_SYSCALL_INTERRUPT_PRIORITY = %d\r\n", configMAX_SYSCALL_INTERRUPT_PRIORITY);						

		if(total_num % 10 == 0) 
		{
			// printf("close interrupt.............\r\n");
			LED0 = 0;
            //close interrupt
			//低于 configMAX_SYSCALL_INTERRUPT_PRIORITY 中断被屏蔽, 无法执行
			//关闭中断之后之后优先级大于 configMAX_SYSCALL_INTERRUPT_PRIORITY 的中断不会被屏蔽
			portDISABLE_INTERRUPTS();           
			delay_xms(5000);
			// printf("\r\nconfigMAX_SYSCALL_INTERRUPT_PRIORITY = %d\r\n", configMAX_SYSCALL_INTERRUPT_PRIORITY);						
			// printf("open interrupt.............\r\n");
            //open interrupt

			portENABLE_INTERRUPTS();
			//删除 UI 线程
			//vTaskDelete(Thread_UI_Handler);
			// printf("delete ui thread.\r\n");
		}
        LED0 = 1;
        vTaskDelay(1000);
    }
} 

static void interrupt_task_exit(void *param)
{
	
}

