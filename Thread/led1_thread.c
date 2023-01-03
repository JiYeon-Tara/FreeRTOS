#include "led1_thread.h"
#include "led.h"
#include "event_groups.h"
#include "thread_manager.h"

/********************
 * MACRO
 ********************/


/********************
 * FUNCTION
 ********************/
static void led1_task(void *pvParameters);
static void led1_task_exit(void *param);


/********************
 * GLOBAL VAR
 ********************/
thread_cb_t led1_thread = {
	.thread_init = led1_task,
	.thread_deinit = led1_task_exit,
};

extern EventGroupHandle_t manager_event_group; //事件标志组, 可以用于一个任务/事件与多个任务/事件进行同步




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
    led1_thread.mutex = xQueueCreateMutex(1);
    configASSERT(led1_thread.mutex);

    led1_thread.queue = xQueueCreate(LED1_QUEUE_SIZE, sizeof(thread_msg_t));
    configASSERT(led1_thread.queue != NULL);

    led1_thread.sema = xSemaphoreCreateBinary();
    configASSERT(led1_thread.sema != NULL);

    led1_thread.event_group = xEventGroupCreate();
    configASSERT(led1_thread.event_group != NULL);
}

//LED1任务入口函数
static void led1_task(void *pvParameters)
{
    taskENTER_CRITICAL();
    hardware_init();
    resource_init();
    software_init();
    printf("thread led1 running...\r\n");
	taskEXIT_CRITICAL();

    while(1)
    {
        // printf("%s running..\r\n", __func__);
        // LED1=0;
        // vTaskDelay(2000);		//延时，当前任务进入阻塞态，调度器进行任务调度
        // LED1=1;
        // vTaskDelay(2000);

        // 消息处理线程阻塞等待事件
        // xEventGroupWaitBits(manager_event_group, TASK_SYNC, pdTRUE, pdTRUE, portMAX_DELAY);
        // LED1 = ~LED1;
        // printf("LED1 thread got a sync bit.\r\n");
    }
	//return; //正常来说执行不到这里
}

static void led1_task_exit(void *param)
{
	
}

