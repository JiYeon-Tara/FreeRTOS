#ifndef __THREAD_KEY_H__
#define __THREAD_KEY_H__

#include "FreeRTOS.h"
#include "task.h"
#include "key.h"

/**************************** global varible ******************************/

/**************************** macro definition ******************************/

/**************************** macro definition ******************************/

//Key Task
#define KEY_TASK_PRIO 2         //任务优先级
#define KEY_STK_SIZE 128        //任务堆栈大小
extern TaskHandle_t KeyTask_Handler;   //任务句柄
void key_task(void *pvParameters); //任务函数

#endif //end __THREAD_KEY_H__

