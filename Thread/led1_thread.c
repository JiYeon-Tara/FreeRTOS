#include "led1_thread.h"
#include "led.h"
#include "event_groups.h"
#include "thread_manager.h"

/**************************** task info ******************************/
TaskHandle_t LED1Task_Handler; //LED0任务句柄

/**************************** global varible ******************************/
extern EventGroupHandle_t manager_event_group; //事件标志组, 可以用于一个任务/事件与多个任务/事件进行同步

/**************************** macro definition ******************************/

/**************************** macro definition ******************************/


/**
 * @brief hardware_init
 * 
 */
static void hardware_init()
{
    LED1 = 1;
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
void led1_task(void *pvParameters)
{
    hardware_init();
    resource_init();
    software_init();

    while(1)
    {
        // printf("%s running..\r\n", __func__);
        // LED1=0;
        // vTaskDelay(2000);		//延时，当前任务进入阻塞态，调度器进行任务调度
        // LED1=1;
        // vTaskDelay(2000);

        // 消息处理线程阻塞等待事件
        xEventGroupWaitBits(manager_event_group, TASK_SYNC, pdTRUE, pdTRUE, portMAX_DELAY);
        LED1 = ~LED1;
        printf("LED1 thread got a sync bit.\r\n");
    }
	//return;					//正常来说执行不到这里
}
