#ifndef __THREAD_DUAL_COMM_H__
#define __THREAD_DUAL_COMM_H__
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "thread.h"





/********************
 * MACRO
 ********************/
#define DUAL_COMM_TASK_PRIO 4         //任务优先级
#define DUAL_COMM_STK_SIZE 128        //任务堆栈大小


#define DUAL_COMM_Q_SIZE            256     //队列大小

// extern TaskHandle_t DualCommTask_Handler;   //任务句柄
// void dual_comm_task(void *pvParameters); //任务函数

/********************
 * GLOBAL VAR
 ********************/
extern thread_cb_t dual_comm_thread;
extern xQueueHandle Dual_Comm_Queue;          //队列句柄
extern SemaphoreHandle_t BinarySemaphore;      //二值信号量

#endif // end __THREAD_DUAL_COMM_H__

