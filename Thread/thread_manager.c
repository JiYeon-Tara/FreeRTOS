/**
 * @file thread_manager.c
 * @author your name (you@domain.com)
 * @brief create thread manager to manage all threads
 * @version 0.1
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "thread_manager.h"
#include "sys.h"
#include "usart.h"
/**************************** task info ******************************/
TaskHandle_t ManagerTask_Handler;   //任务句柄

// 对所有线程进行管理
// 所有线程的父线程, 所有线程的初始化以及退出
/**************************** global varible ******************************/
EventGroupHandle_t manager_event_group; //事件标志组, 可以用于一个任务/事件与多个任务/事件进行同步

/**************************** macro definition ******************************/

/**************************** macro definition ******************************/

/**
 * @brief hardware_init
 * 
 */
static void hardware_init()
{

    return;
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
    // event group
    manager_event_group = xEventGroupCreate();
}

/**
 * @brief 线程入口函数, manager 线程中对所有线程进行初始化
 * 
 * @param pvParameters 
 */
void thread_manager_task(void *pvParameters)
{
    hardware_init();
    resource_init();
    software_init();

    while(1)
    {
        printf("manager thread running...\r\n");
        vTaskDelay(1000);
    }
}

