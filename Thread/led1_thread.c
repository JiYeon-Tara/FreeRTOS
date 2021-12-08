#include "led1_thread.h"
#include "led.h"

/**************************** global varible ******************************/

/**************************** macro definition ******************************/

/**************************** macro definition ******************************/

//LED1任务句柄
TaskHandle_t LED1Task_Handler;

//LED1任务入口函数
void led1_task(void *pvParameters)
{
    while(1)
    {
        printf("%s running..\r\n", __func__);
        LED1=0;
        vTaskDelay(2000);		//延时，当前任务进入阻塞态，调度器进行任务调度
        LED1=1;
        vTaskDelay(2000);

    }
	//return;					//正常来说执行不到这里
}
