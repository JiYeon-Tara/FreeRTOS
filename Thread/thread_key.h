#ifndef __THREAD_KEY_H__
#define __THREAD_KEY_H__

#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "thread.h"

/********************
 * MACRO
 ********************/
#define KEY_TASK_PRIO 5         //
#define KEY_STK_SIZE 128        //
// extern TaskHandle_t KeyTask_Handler;   //
// void key_task(void *pvParameters); //
#define KEY_Q_SIZE   20


/********************
 * GLOBAL VAR
 ********************/
extern thread_cb_t key_thread;

extern xQueueHandle Key_Queue;          //
extern xSemaphoreHandle key_sema;      //





#endif //end __THREAD_KEY_H__

