#include "led0_thread.h"
#include "str_operation.h"
#include "command_parse.h"
#include "usart.h"  //需要用到串口中的全局变量
#include "event_groups.h"
#include "thread_manager.h"
//#include "timer.h"
/**************************** task info ******************************/
TaskHandle_t LED0Task_Handler; //LED0任务句柄

/**************************** global varible ******************************/
extern SemaphoreHandle_t BinarySemaphore;      //二值信号量 -> 会导致"优先级翻转", 使用互斥信号量
extern xSemaphoreHandle key_sema;      //计数型信号量
extern EventGroupHandle_t manager_event_group; //事件标志组, 可以用于一个任务/事件与多个任务/事件进行同步

/**************************** macro definition ******************************/

/**************************** macro definition ******************************/

/**
 * @brief hardware_init
 * 
 */
static void hardware_init()
{
    LED0 = 0;
}

/**
 * @brief software_init
 * 
 */
static void software_init()
{
    // //通知其它线程初始化 -> 通常使用任务通知实现
    // //设置标志位
    // xEventGroupSetBits(manager_event_group, TASK_SYNC);
}

/**
 * @brief resource_init
 * 
 */
static void resource_init()
{

}

//LED1任务入口函数
void led0_task(void *pvParameters)
{
    uint8_t len = 0;
    uint8_t CommandVal = COMMAND_ERR;
    uint8_t *pCommandStr;
    uint8_t semaCnt;
    hardware_init();
    resource_init();
    software_init();

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

        //
        //阻塞获取计数型信号量
        //
        // xSemaphoreTake(key_sema, portMAX_DELAY);
        // semaCnt = uxSemaphoreGetCount(key_sema);
        // printf("led thread take semapthore, sema cnt:%d", semaCnt);

        // 消息处理线程阻塞等待事件
        xEventGroupWaitBits(manager_event_group, TASK_SYNC, pdTRUE, pdTRUE, portMAX_DELAY);
        LED0 = ~LED0;
        printf("LED0 thread got a sync bit.\r\n");
        
    }
	//return;					//正常来说执行不到这里
} 


