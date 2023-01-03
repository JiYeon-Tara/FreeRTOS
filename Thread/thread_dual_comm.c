/**
 * @file thread_dual_comm.c
 * @author your name (you@domain.com)
 * @brief PC 从串口发送数据, STM32 进行解析
 * @version 0.1
 * @date 2021-12-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "thread_dual_comm.h"
#include "thread.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "usart.h"
#include "my_timer.h"
#include "led0_thread.h"

/********************
 * MACRO
 ********************/
//用于串口控制 LED 的命令
#define LED0_ON     1
#define LED0_OFF    2
#define LED1_ON     3
#define LED1_OFF    4
#define COMMAND_ERR 0xFF

/********************
 * FUNCTION
 ********************/
void dual_comm_task(void *pvParameters);
static void dual_comm_task_exit(void *param);


/********************
 * GLOBAL VAR
 ********************/
thread_cb_t dual_comm_thread = {
	.thread_init = dual_comm_task,
	.thread_deinit = dual_comm_task_exit,
};

TaskHandle_t DualCommTask_Handler;      //任务句柄
xQueueHandle Dual_Comm_Queue;           //消息队列句柄
SemaphoreHandle_t BinarySemaphore;      //二值信号量 -> 会导致"优先级翻转", 使用互斥信号量
xTimerHandle g_timer_handle;            //定时器句柄


/**
 * @brief hardware_init
 * 
 */
static void hardware_init()
{
	//uart_init(115200);					//初始化串口

    return;
}

/**
 * @brief software_init
 * 
 */
static void software_init()
{

}

/**
 * @brief resource_init
 * 
 */
static void resource_init()
{
    Dual_Comm_Queue = xQueueCreate(DUAL_COMM_Q_SIZE, sizeof(uint8_t));  
    configASSERT(Dual_Comm_Queue != NULL);

    BinarySemaphore = xSemaphoreCreateBinary();
    configASSERT(BinarySemaphore != NULL);

    //初始化定时器
    // g_timer_handle = xTimerCreate((const char*)"periodic_timer",
    //                               (TickType_t)1000,
    //                               (UBaseType_t)pdTRUE,
    //                               (void*)1,
    //                               (TimerCallbackFunction_t)periodic_timer_cb);
    // xTimerStart(g_timer_handle, 0);
}

/**
 * @brief key_task
 * 
 * @param pvParameters 
 */
void dual_comm_task(void *pvParameters)
{
    uint8_t t = 0;
    taskENTER_CRITICAL();
    hardware_init();
    software_init();
    resource_init();
    printf("thread dual comm running...\r\n");
	taskEXIT_CRITICAL();

    while(1)
    {
        //printf("dualcomm task running...\r\n");

        //这个是 轮询的方式, 这个方式不行 -> 串口中断
        if(USART_RX_STA & 0x8000)
		{
			uint16_t len = USART_RX_STA & 0x3fff;  //得到接收的数据长度

			printf("received:\r\n");
			for(t = 0; t < len; ++t)
			{
				USART1->DR = USART_RX_BUF[t];
				while((USART1->SR & 0X40) == 0); //等待数据发送结束
                //printf("%c", )
			}

            // 收到的数据发送到队列中, 一次发送大量数据
            // xQueueSend(Dual_Comm_Queue, USART_RX_BUF, 10);
            if(!thread_msg_send(&led0_thread, THREAD_LED0_MSG_ID_DATA, (void*)USART_RX_BUF, len)){
                printf("thread dual comm msg send failed, msg_id:%d, msg_len:%d\r\n", 0, len);
            }

            // 通过 semaphore 发送到 thread_led0 线程
            // 应该是串口中断中释放信号量: dual_comm_thread 中进行解析
            // if(BinarySemaphore != NULL)
            // {
            //     xSemaphoreGive(BinarySemaphore);
            // }
            

			USART_RX_STA = 0;   //复位标志位
            memset(USART_RX_BUF, 0, USART_REC_LEN); //清除全局变量缓存
		}

        // vTaskDelay(10); //延时 10ms
    }
}

static void dual_comm_task_exit(void *param)
{
	
}

