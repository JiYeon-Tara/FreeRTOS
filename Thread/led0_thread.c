#include "led0_thread.h"
#include "led.h"

//LED0任务句柄
TaskHandle_t LED0Task_Handler;

//LED1任务入口函数
void led0_task(void *pvParameters)
{
    while(1)
    {
        LED0=~LED0;
		//这里延时不是500 ms 而是根据 FreeRTOSConfig.h 中配置来决定的
		//#define configTICK_RATE_HZ						(1000)   
		//所以500次， 就是500ms
        printf("%s running..\r\n", __func__);
        vTaskDelay(500);		//延时，当前任务进入阻塞态，调度器进行任务调度
    }
	return;					//正常来说执行不到这里
} 


