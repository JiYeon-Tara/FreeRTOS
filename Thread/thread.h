/**
 * @file thread.h
 * @author your name (you@domain.com)
 * @brief all thread information and public macro defination
 * @version 0.1
 * @date 2022-01-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __THREAD_H__
#define __THREAD_H__
#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include <stdbool.h>

/********************
 * MACRO
 ********************/
#define THREAD_MSG_BUFF_SIZE 1024
#define DEBUG                1

#if DEBUG
#define ASSERT(x)   {if(!x) while(1);}
#else
#define ASSERT(x)
#endif

#define SYSTEM_TASK_NOTIFY_SYNC                 (0x80000000)
#define SYSTEM_TASK_NOTIFY_MSG_READY            (0x40000000)

// thread message block
typedef struct {
    uint8_t msg_id; // ͨ����ͬ ID �����ֲ�ͬ������Ӧ�ò���Ϣ
    uint16_t msg_len;
} head_t;

typedef struct {
    head_t head;
    uint8_t data[THREAD_MSG_BUFF_SIZE];
} thread_msg_t;

typedef enum{
    THREAD_MSG_SYNC,
    THREAD_MSG_EXIT,
    THREAD_MSG_DATA,
    THREAD_MSG_UI,
} THREAD_MSG_E;


/**
 * @brief 可以使用一个结构体(类)对每一个任务的这些资源进行管理
 *        后期可以修改
 *        OOP - Object orienter programing
 */
typedef struct thread_control_block {
    TaskFunction_t thread_init;         // thread enter function
    TaskFunction_t thread_deinit;       // thread exit
    TaskHandle_t taskHandle;
    QueueHandle_t mutex;
    QueueHandle_t queue;
    SemaphoreHandle_t sema;
    EventGroupHandle_t event_group;
    // osPoolId pool;                   // 每个线程有自己的内存池
    // void *private_data;
} thread_cb_t;

/********************
 * GLOBAL VAR
 ********************/

/********************
 * function
 ********************/
bool thread_msg_send(const thread_cb_t *pThreadCb, uint8_t msg_id, void *data, uint16_t msg_len);

#endif

