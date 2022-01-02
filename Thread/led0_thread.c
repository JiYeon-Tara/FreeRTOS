#include "led0_thread.h"
#include "semphr.h"
#include "str_operation.h"
#include "command_parse.h"
#include "usart.h"  //需要用到串口中的全局变量
//#include "timer.h"
/**************************** global varible ******************************/
extern SemaphoreHandle_t BinarySemaphore;      //二值信号量

/**************************** macro definition ******************************/

/**************************** macro definition ******************************/


//LED0任务句柄
TaskHandle_t LED0Task_Handler;

//LED1任务入口函数
void led0_task(void *pvParameters)
{
    uint8_t len = 0;
    uint8_t CommandVal = COMMAND_ERR;
    uint8_t *pCommandStr;
    while(1)
    {
        // LED0=~LED0;
		// //这里延时不是500 ms 而是根据 FreeRTOSConfig.h 中配置来决定的
		// //#define configTICK_RATE_HZ						(1000)   
		// //所以500次， 就是500ms
        // printf("%s running..\r\n", __func__);
        // vTaskDelay(500);		//延时，当前任务进入阻塞态，调度器进行任务调度

        //
        // 对来自 dual_comm 线程的数据进行解析
        //
        // uint8_t err;
        // if(BinarySemaphore != NULL)
        // {
        //     err = xSemaphoreTake(BinarySemaphore, portMAX_DELAY);
        //     if(err == pdTRUE)
        //     {
                
        //     }
        // }
    }
	//return;					//正常来说执行不到这里
} 


