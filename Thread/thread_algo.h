#ifndef __THREAD_ALOG_H__
#define __THREAD_ALOG_H__
#include "FreeRTOS.h"
#include "task.h"
#include "thread.h"



// #define configSUPPORT_STATIC_ALLOCATION 1           //֧�־�̬��������

/********************
 * MACRO
 ********************/
#define     THREAD_ALGO_PRIO 2          //�������ȼ�
#define     THREAD_ALGO_STK_SIZE 128    //�����ջ��С

extern StackType_t ThreadAlgoStack[THREAD_ALGO_STK_SIZE]; //�����ջ
extern StaticTask_t ThreadAlgoTCB;             //������ƿ�
extern TaskHandle_t ThreadAlgo_Handler;        //������
// void thread_algo_enery(void *pvParameters); //������
// extern TaskHandle_t ThreadAlgo_Handler;        //������

/********************
 * GLOBAL VAR
 ********************/
extern thread_cb_t algo_thread;

#endif //end __THREAD_ALOG_H__

