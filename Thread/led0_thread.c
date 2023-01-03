#include "led0_thread.h"
#include "str_operation.h"
#include "command_parse.h"
#include "usart.h"  //需要用到串口中的全局变量
#include "FreeRTOS.h"
#include "event_groups.h"
#include "thread_manager.h"
//#include "timer.h"


/********************
 * MACRO
 ********************/


/********************
 * FUNCTION
 ********************/
static void led0_task(void *pvParameters);
static void led0_task_exit(void *param);
static void _thread_service_msg_process();


/********************
 * GLOBAL VAR
 ********************/
thread_cb_t led0_thread = {
	.thread_init = led0_task,
	.thread_deinit = led0_task_exit,
};

extern SemaphoreHandle_t BinarySemaphore;      //二值信号量 -> 会导致"优先级翻转", 使用互斥信号量
extern xSemaphoreHandle key_sema;      //计数型信号量
extern EventGroupHandle_t manager_event_group; //事件标志组, 可以用于一个任务/事件与多个任务/事件进行同步



/**
 * @brief hardware_init
 * 
 */
static void hardware_init()
{
    LED_Init();
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
    led0_thread.mutex = xQueueCreateMutex(1);
    configASSERT(led0_thread.mutex);

    // printf("sizeof(thrad_msg_t) = %d\r\n", sizeof(thrad_msg_t));
    led0_thread.queue = xQueueCreate(LED0_QUEUE_SIZE, sizeof(thread_msg_t));
    configASSERT(led0_thread.queue != NULL);

    led0_thread.sema = xSemaphoreCreateBinary();
    configASSERT(led0_thread.sema != NULL);

    led0_thread.event_group = xEventGroupCreate();
    configASSERT(led0_thread.event_group != NULL);
}

//LED1任务入口函数
static void led0_task(void *pvParameters)
{
//    uint8_t len = 0;
//    uint8_t CommandVal = COMMAND_ERR;
//    uint8_t *pCommandStr;
//    uint8_t semaCnt;
    uint32_t event;
    BaseType_t ret;
	taskENTER_CRITICAL();
    hardware_init();
    resource_init();
    software_init();
    printf("thread led0 running...\r\n");
	taskEXIT_CRITICAL();

    while(1)
    {
        event = 0;
        ret = xTaskNotifyWait(0, 0xFFFFFFFF, &event, portMAX_DELAY);
        if(ret == pdFALSE){ // 或者等待超时, 可以设置最长等待超时时间

        }

        if(event & SYSTEM_TASK_NOTIFY_MSG_READY){
            _thread_service_msg_process();
        }
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
        // xEventGroupWaitBits(manager_event_group, TASK_SYNC, pdTRUE, pdTRUE, portMAX_DELAY);
        // LED0 = ~LED0;
        // printf("LED0 thread got a sync bit.\r\n");
        
    }
	//return;					//正常来说执行不到这里
} 

static void led0_task_exit(void *param)
{
	
}

/**
 * @brief get message from message queue
 * 
 */
static void _thread_service_msg_process()
{
    BaseType_t ret;
    thread_msg_t *p_msg;

    while(1){
        ret = xQueueReceive(led0_thread.queue, &p_msg, 0);
        if(ret != pdPASS || p_msg == NULL){
            break;
        }
        printf("data received msg_id:%d msg_len:%d\r\n", p_msg->head.msg_id, p_msg->head.msg_len);
        switch(p_msg->head.msg_id){
            case THREAD_LED0_MSG_ID_RAW:
            {

            }
            break;
            case THREAD_LED0_MSG_ID_DATA:
            {
            }
            break;
            default:
            break;
        }
        free(p_msg); // free memeory
        p_msg = NULL;
    }

    return;
}