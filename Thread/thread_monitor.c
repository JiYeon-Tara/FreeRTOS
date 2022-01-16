/**
 * @file thread_monitor.c
 * @author your name (you@domain.com)
 * @brief 管理 AT 命令等的线程, FI 命令等
 * @version 0.1
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "thread_monitor.h"
#include "sys.h"
#include "str_operation.h"
#include "string.h"
#include "usart.h"
//#include "memory.h"
#include "led.h"
#include "beep.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief 可以使用一个结构体(类)对每一个任务的这些资源进行管理
 *        后期可以修改
 */
/**************************** task info ******************************/
StackType_t ThreadMonitorStack[THREAD_MONITOR_STK_SIZE]; //任务堆栈
StaticTask_t ThreadMonitorTCB;             //任务控制块
TaskHandle_t ThreadMonitor_Handler;        //任务句柄
SemaphoreHandle_t monitor_binary_handle;    //二值信号量

/**************************** global varible ******************************/


/**************************** macro definition ******************************/
//串口命令解析
#define LED1ON          1
#define LED1OFF         2
#define BEEPON          3
#define BEEPOFF         4
#define COMMAND_ERR     0xFF

/**************************** macro definition ******************************/
static void hardware_init()
{

}

static void software_init()
{
   
}

static void resource_init()
{
    monitor_binary_handle = xSemaphoreCreateBinary();
}

/**
 * @brief monitor thread entry
 * 
 * @param pvParameters 
 */
void thread_monitor_enery(void *pvParameters)
{
    BaseType_t ret = pdFALSE;
    uint8_t len = 0;    //接收到数据长度
    uint8_t cmdVal = COMMAND_ERR;
    //uint8_t *cmdStr = NULL; //指向接收到数据的指针
    uint8_t cmdStr[256];
    
    hardware_init();
    resource_init();
    software_init();
    while(1)
    {
        printf("thread_moonitor running...");
        vTaskDelay(1000);
        /*if(monitor_binary_handle != NULL)
        {
            //
            // 阻塞等待信号量
            //
            ret = xSemaphoreTake(monitor_binary_handle, portMAX_DELAY);
            if(ret == pdTRUE)
            {
                len = USART_RX_STA & 0x3FFF;
                //
                // 一动态分配内存就报:什么半主机的错误
                //
                //cmdStr = (uint8_t*)malloc(len + 1);   //字符串结尾 '\0'
                sprintf((char*)cmdStr, "%s", USART_RX_BUF); //格式转换为字符串
                cmdStr[len] = '\0';
                LowerToCapital(cmdStr, len);
                cmdVal = CommandParse((char*)cmdStr);  //命令解析
                if(cmdVal != COMMAND_ERR)
                {
                    //
                    // 给 GUI 线程队列塞消息
                    //
                    //printf("cmd:%d", cmdVal);
                }
                else
                {

                }

                //
                // 根据命令进行相应操作
                //
                switch(cmdVal)
                {
                    case LED1ON:
                        LED1 = 1;
                        break;
                    case LED1OFF:
                        LED1 = 0;
                        break;
                    case BEEPON:
                        BEEP = 1;
                        break;
                    case BEEPOFF:
                        BEEP = 0;
                        break;
                    default:
                        break;
                }
                USART_RX_STA = 0;
                memset(USART_RX_BUF, 0, USART_REC_LEN); //清零缓冲区
                free(cmdStr);
            }
            else
            {
                vTaskDelay(10); //等待 10 个节拍
            }
        }*/
    }
}

/**
 * @brief 命令解析函数
 * 
 * @param str 输入字符串
 */
uint8_t CommandParse(char *str)
{
    uint8_t cmdVal = COMMAND_ERR;
    if( strcmp((char*)str, "LED1ON") == 0){
        cmdVal = LED1ON;
    }
    else if(strcmp((char*)str, "LED1OFF") == 0){
        cmdVal = LED1OFF;
    }
    else if(strcmp((char*)str, "BEEPON") == 0){
        cmdVal = BEEPON;
    }
    else if(strcmp((char*)str, "BEEPOFF") == 0){
        cmdVal = BEEPOFF;
    }
    else{
        //printf("unknow cmd:%s", str);
    }

    return cmdVal;
}

//AT 命令解析是将字符串放到一个数组, 数组元素 char*, pFun
// typedef void (*pFun)(const char *para1, const char *para2);
// typedef struct 
// {
//     char *cmd;
//     pFun fun;
// };

// typedef void (*pFun)(void);
// {
//     .init = pFun;
//     .deinit = deinit;
// }
