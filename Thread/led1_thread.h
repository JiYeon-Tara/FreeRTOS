#ifndef __LED1_THREAD_H__
#define __LED1_THREAD_H__
#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"

/**************************** 根据B520项目经验对 FreeRTOS 模板代码进行重构 **********************************/
/*
* B520初始化所有任务的方法，申明全局变量，并且初始化好每一个参数到一个任务控制块的结构体中
* 这样在管理任务中只需要调用: led0_thread->init() 即可
* 充分实现了代码之间的低耦合， 高内聚
*/
//led1任务 相关的宏定义
//任务优先级
#define LED1_TASK_PRIO		3
//任务堆栈大小	
#define LED1_STK_SIZE 		50  
//任务句柄， 全局变量
extern TaskHandle_t LED1Task_Handler;
//任务函数
void led1_task(void *pvParameters);


#endif // end __LED1_THREAD_H__

