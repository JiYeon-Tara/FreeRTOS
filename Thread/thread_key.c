#include "thread_key.h"

/**************************** global varible ******************************/
TaskHandle_t KeyTask_Handler;   //任务句柄
xQueueHandle Key_Queue;          //消息队列句柄

/**************************** macro definition ******************************/

/**************************** macro definition ******************************/

/**
 * @brief hardware_init
 * 
 */
static void hardware_init()
{
    KEY_Init();

    return;
}

/**
 * @brief software_init
 * 
 */
static void software_init()
{

}

/**
 * @brief resource_init
 * 
 */
static void resource_init()
{
    Key_Queue = xQueueCreate(KEY_Q_SIZE, sizeof(uint8_t));  //初始化消息队列
}

/**
 * @brief key_task
 * 
 * @param pvParameters 
 */
void key_task(void *pvParameters)
{
    uint8_t key = 0;
    hardware_init();
    software_init();
    resource_init();

    while(1)
    {
        //printf("Key thread running...\r\n");
        //在有操作系统的地方使用 轮询按键有问题
        key = KEY_Scan(0);

        //发送
        //printf("thread_key = %d\r\n", key);
        if(Key_Queue!=NULL && key!=0)
        {
            xQueueSend(Key_Queue, &key, 1000);   //发送到消息队列
        }
        switch(key)
        {
            //按键 WKUP 坏了
            //导致按键一直往这里跑, 其它按键也无效了
            case WKUP_PRES:
                //vTaskSuspend(Task1Task_Handler); //挂起任务 1 (2)
                //printf("挂起任务 1 的运行!\r\n");
                break;
            case KEY0_PRES:
                //vTaskResume(Task1Task_Handler); //恢复任务 1 (3)
                printf("恢复任务 1 的运行!\r\n");
                break;
            case KEY1_PRES:
                //vTaskSuspend(Task2Task_Handler);//挂起任务 2 (4)
                printf("挂起任务 2 的运行!\r\n");
                break;
            default:
                //printf("%s No keyboard input.\r\n", __func__);
                break;
        }
        vTaskDelay(10); //延时 10ms
    }
}
