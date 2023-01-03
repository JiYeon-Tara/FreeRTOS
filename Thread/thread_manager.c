/**
 * @file thread_manager.c
 * @author your name (you@domain.com)
 * @brief create thread manager to manage all threads
 * @version 0.1
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "thread_manager.h"
#include "sys.h"
#include "usart.h"
#include "test_config.h"
// #include "rtc.h"

/********************
 * MACRO
 ********************/


/********************
 * FUNCTION
 ********************/
void thread_manager_task(void *pvParameters);
static void manager_task_exit(void *param);


/********************
 * GLOBAL VAR
 ********************/
thread_cb_t manager_thread = {
	.thread_init = thread_manager_task,
	.thread_deinit = manager_task_exit,
};

EventGroupHandle_t manager_event_group;


/**
 * @brief hardware_init
 * 
 */
static void hardware_init()
{
    printf("%s\r\n", __func__);
    // while(RTC_Init()); // �ȴ� RTC ��ʼ���ɹ�
    // RTC_Set(2022, 4, 23, 8, 47, 0);
    return;
}

/**
 * @brief software_init
 * 
 */
static void software_init()
{
    // xEventGroupSetBits(manager_event_group, TASK_SYNC);
#if LIST_TEST_ENABLE
    list_test();
#endif
}

/**
 * @brief resource_init
 * 
 */
static void resource_init()
{
    // event group
    manager_event_group = xEventGroupCreate();
}

/**
 * @brief �߳���ں���, manager �߳��ж������߳̽��г�ʼ��
 * 
 * @param pvParameters 
 */
void thread_manager_task(void *pvParameters)
{
    taskENTER_CRITICAL();
    hardware_init();
    resource_init();
    software_init();
    printf("thread manager running...\r\n");
	taskEXIT_CRITICAL();

    while(1)
    {
        // Disp_Time(0, 0, 0); // ���Է���Ϣ�� UI �߳��� UI ��ʾ
        vTaskDelay(1000);
    }
}

static void manager_task_exit(void *param)
{
	
}

