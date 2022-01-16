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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "usart.h"
#include "my_timer.h"

/**************************** global varible ******************************/
TaskHandle_t DualCommTask_Handler;      //任务句柄
xQueueHandle Dual_Comm_Queue;           //消息队列句柄
SemaphoreHandle_t BinarySemaphore;      //二值信号量 -> 会导致"优先级翻转", 使用互斥信号量
xTimerHandle g_timer_handle;            //定时器句柄
/**************************** macro definition ******************************/
//用于串口控制 LED 的命令
#define LED0_ON     1
#define LED0_OFF    2
#define LED1_ON     3
#define LED1_OFF    4
#define COMMAND_ERR 0xFF

/**************************** macro definition ******************************/

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
    //
    // initialize queue
    //
    Dual_Comm_Queue = xQueueCreate(DUAL_COMM_Q_SIZE, sizeof(uint8_t));  

    //
    // 初始化二值信号量
    //
    BinarySemaphore = xSemaphoreCreateBinary();

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
    hardware_init();
    software_init();
    resource_init();

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
            //
            // 收到的数据发送到队列中
            // 一次发送大量数据
            //
            xQueueSend(Dual_Comm_Queue, USART_RX_BUF, 10);

            //
            // 通过 semaphore 发送到 thread_led0 线程
            // 应该是串口中断中释放信号量: dual_comm_thread 中进行解析
            //
            if(BinarySemaphore != NULL)
            {
                xSemaphoreGive(BinarySemaphore);
            }
            

			USART_RX_STA = 0;   //复位标志位
            memset(USART_RX_BUF, 0, USART_REC_LEN); //清除全局变量缓存
		}

        vTaskDelay(10); //延时 10ms
    }
}

