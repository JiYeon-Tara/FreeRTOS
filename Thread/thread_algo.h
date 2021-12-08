#ifndef __THREAD_ALOG_H__
#define __THREAD_ALOG_H__
#include "FreeRTOS.h"
#include "task.h"


/**************************** global varible ******************************/

/**************************** macro definition ******************************/

/**************************** macro definition ******************************/

//
// thread algo 
// 使用静态方法创建任务时, 需要打开宏:
// #define configSUPPORT_STATIC_ALLOCATION 1           //支持静态创建任务
//
#define     THREAD_ALGO_PRIO 2          //任务优先级
#define     THREAD_ALGO_STK_SIZE 128    //任务堆栈大小
extern StackType_t ThreadAlgoStack[THREAD_ALGO_STK_SIZE]; //任务堆栈
extern StaticTask_t ThreadAlgoTCB;             //任务控制块
extern TaskHandle_t ThreadAlgo_Handler;        //任务句柄
void thread_algo_enery(void *pvParameters); //任务函数
extern TaskHandle_t ThreadAlgo_Handler;        //任务句柄


#endif //end __THREAD_ALOG_H__

