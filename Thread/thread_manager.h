/**
 * @file thread_manager.h
 * @author your name (you@domain.com)
 * @brief 线程中有关的宏以及需要对外暴露的接口都放在 .h 中
 * @version 0.1
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __THREAD_MANAGER_H__
#define __THREAD_MANAGER_H__
#include "FreeRTOS.h"
#include "event_groups.h"
/**************************** task info ******************************/
#define MANAGER_TASK_PRIO 5         //任务优先级
#define MANAGER_STK_SIZE 128        //任务堆栈大小
extern TaskHandle_t ManagerTask_Handler;   //任务句柄
void thread_manager_task(void *pvParameters); //任务函数

/**************************** global varible ******************************/
extern EventGroupHandle_t event_group; //事件标志组, 可以用于一个任务/事件与多个任务/事件进行同步

/**************************** macro definition ******************************/
#define LED0_EVENT      (1 << 0) //0x00000001
#define LED1_EVENT      (1 << 1)
#define KET_EVENT       (1 << 2)
#define GUI_EVENT       (1 << 3)

#define TASK_SYNC       (1 << 4)
#define TASK_EXIT       (1 << 5)
#define EVENT_BIT_ALL    LED0_EVENT | LED1_EVENT | KET_EVENT | GUI_EVENT
/**************************** macro definition ******************************/


#endif // end __THREAD_MANAGER_H__

