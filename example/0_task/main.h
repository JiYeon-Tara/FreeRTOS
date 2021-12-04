#ifndef __MAIN_H__
#define __MAIN_H__

#include "led.h"
#include "delay.h"
#include "sys.h"
#include "FreeRTOS.h"		//必须把FreeRTOS.h 放在 task.h 文件的前面包含
#include "task.h"

#include "timer.h"
#include "my_timer.h"

#include "led0_thread.h"
#include "led1_thread.h"
#include "interrupt_thread.h"

//使用开始任务中创建两个任务, 
//eg: B520项目中的 thread_manager 线程 -> 可以使用任务控制块对任务进行管理
//管理任务相关设置
#define START_TASK_PRIO		1       //任务优先级
#define START_STK_SIZE 		128     //任务堆栈大小	
TaskHandle_t StartTask_Handler;     //任务句柄
void start_task(void *pvParameters);//任务入口函数



#endif // end __MAIN_H__

