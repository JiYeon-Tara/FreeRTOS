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


/**************************** global varible ******************************/
TaskHandle_t DualCommTask_Handler;   //任务句柄
xQueueHandle Dual_Comm_Queue;          //消息队列句柄

/**************************** macro definition ******************************/

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
    Dual_Comm_Queue = xQueueCreate(DUAL_COMM_Q_SIZE, sizeof(uint8_t));  //initialize queue


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

			USART_RX_STA = 0;   //复位标志位
            memset(USART_RX_BUF, 0, USART_REC_LEN); //清除全局变量缓存
		}

        vTaskDelay(10); //延时 10ms
    }
}

