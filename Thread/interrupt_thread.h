#ifndef __INTERRUPT_THREAD_H__
#define __INTERRUPT_THREAD_H__
#include "FreeRTOS.h"
#include "task.h"
#include "thread.h"

/********************
 * MACRO
 ********************/
#define INTERRUPT_TASK_PRIO		        2
#define INTERRUPT_STK_SIZE 		        256  
// extern TaskHandle_t INTERRUPTTask_Handler;  //global thread handler declaration


/********************
 * GLOBAL VAR
 ********************/
extern thread_cb_t interrupt_thread;

#endif //end __INTERRUPT_THREAD_H__

