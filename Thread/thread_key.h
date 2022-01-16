#ifndef __THREAD_KEY_H__
#define __THREAD_KEY_H__

#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
/**************************** task info ******************************/
#define KEY_TASK_PRIO 5         //任务优先级
#define KEY_STK_SIZE 128        //任务堆栈大小
extern TaskHandle_t KeyTask_Handler;   //任务句柄
void key_task(void *pvParameters); //任务函数

/**************************** global varible ******************************/
extern xQueueHandle Key_Queue;          //消息队列句柄
extern xSemaphoreHandle key_sema;      //计数型信号量
/**************************** macro definition ******************************/
//queue size
#define KEY_Q_SIZE   20

/**************************** macro definition ******************************/

//Key Task


#endif //end __THREAD_KEY_H__

