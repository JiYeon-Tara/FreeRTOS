#ifndef __LED0_THREAD_H__
#define __LED0_THREAD_H__
#include "sys.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "led.h"

/**************************** global varible ******************************/

/**************************** macro definition ******************************/

/**************************** macro definition ******************************/

//LED task
#define LED0_TASK_PRIO		2 //任务优先级
#define LED0_STK_SIZE 		50  //任务堆栈大小	
extern TaskHandle_t LED0Task_Handler; //任务句柄, 声明为全局变量
void led0_task(void *pvParameters); //任务函数

/**
* @fun: 对于任务(线程)可以使用全局变量的方式
* 结构体
void (*funPtr)(void);	//函数指针
typedef struct TASK{
	funPtr taskEnterFun;
	char * taskName;
	int stackSize;
	int stackPrioirity;
	funPtr taskEnd;
	//与消息队列有关的成员
	message_t	taskMessage;
	queue_t		takQueue;
	mail_t		taskMail;
	semaphare_t	taskSemaphere;
	//等等，一个任务有关结构体中可以包含所有有关的成员
} Task;
* @desc: B520项目，它是把这个任务结构体放在全局变量中, .c 文件中就初始化
*		 main 还没执行，这些任务控制块 init, delete 已经初始化好了
*		 在 manager_thread 任务中再调用 task1.init() 初始化线程，调用线程入口函数
*/

#endif

