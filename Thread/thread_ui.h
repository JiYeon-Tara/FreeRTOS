#ifndef __THREAD_UI_H__
#define __THREAD_UI_H__

#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/**************************** global varible ******************************/

/**************************** macro definition ******************************/

/**************************** macro definition ******************************/

//thread ui task
#define THREAD_UI_PRIO 3 //任务优先级
#define THREAD_UI_STK_SIZE 128 //任务堆栈大小
extern TaskHandle_t Thread_UI_Handler; //任务句柄
void thread_ui_entry(void *pvParameters); //任务函数

#endif

