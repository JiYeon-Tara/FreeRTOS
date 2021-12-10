/**
 * @file thread_ui.c
 * @author your name (you@domain.com)
 * @brief start GUI thread use LCD 
 * @version 0.1
 * @date 2021-12-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "thread_ui.h"
#include <stdint.h>
#include "led.h"
#include "lcd.h"
#include <stdint.h>
#include <string.h>

/**************************** global varible ******************************/
//LCD 刷屏时使用的颜色
static int lcd_discolor[14] = { WHITE, BLACK, BLUE, BRED, 
                        GRED, GBLUE, RED, MAGENTA, 
                        GREEN, CYAN, YELLOW, BROWN, 
                        BRRED, GRAY };
extern xQueueHandle Key_Queue;          //消息队列句柄
extern xQueueHandle Dual_Comm_Queue;          //消息队列句柄
/**************************** macro definition ******************************/

/**************************** macro definition ******************************/

//thread ui task
TaskHandle_t Thread_UI_Handler; //任务句柄

static void thread_ui_hardware_init()
{
    LCD_Init();         //初始化 LCD

    return;
}


static void thread_ui_software_init()
{
    POINT_COLOR = RED;
    LCD_ShowString(30, 10, 200, 16, 16, "ATK STM32F103/F407");
    LCD_ShowString(30, 30, 200, 16, 16, "FreeRTOS Examp 6-1");
    LCD_ShowString(30, 50, 200, 16, 16, "Task Creat and Del");
    LCD_ShowString(30, 70, 200, 16, 16, "ATOM@ALIENTEK");
    LCD_ShowString(30, 90, 200, 16, 16, "2016/11/25");

    return;
}

void thread_ui_entry(void *pvParameters)
{
    uint8_t num = 0;

    thread_ui_hardware_init();
    thread_ui_software_init();
    POINT_COLOR = BLACK;
    LCD_DrawRectangle(5, 110, 115, 314); //画一个矩形
    LCD_DrawLine(5, 130, 115, 130); //画线
    POINT_COLOR = BLUE;
    LCD_ShowString(6, 111, 110, 16, 16, "Task1 Run:000");

    while(1)
    {
        
        //
        // 阻塞接收来自按键线程的消息
        //
        if(Key_Queue != NULL)
        {
            uint8_t key_val;
            //
            // 要通过 xQueueReceive 的返回值进行判断
            //
            if(xQueueReceive(Key_Queue, &key_val, 10) == pdTRUE) //portMAX_DELAY
            {
                printf("ui_thread receive key val = %d\r\n", key_val);
                //UI show string
                //LCD_fill();
            }
        }

        //
        // queue接收来自Dual comm 线程的消息
        //
        if(Dual_Comm_Queue != NULL)
        {
            uint8_t uart_receive[200];
            
            //
            // 每次只能读取一个字节 ???????????????
            //
            if(xQueueReceive(Dual_Comm_Queue, uart_receive, 10))   //一次只能读取一个字节吗？？？
            {
                uint8_t ix = 0;
                printf("ui_thread receive from usart:\r\n");
                for(ix = 0; ix < 200; ++ix)
                    printf("%c", uart_receive[ix]);
            }
        }

        num++; //任务执 1 行次数加 1 注意 num 加到 255 的时候会清零！！
        //LED0 =! LED0;
        printf("任务 1 已经执行：%d 次\r\n", num);
        if(num==5) 
        {
            //vTaskDelete(Task2Task_Handler);//任务 1 执行 5 次删除任务 2 (4)
            //printf("任务 1 删除了任务 2!\r\n");
        }
        LCD_Fill(6, 131, 114, 313, lcd_discolor[num % 14]); //填充区域
        //LCD_Fill(6, 131, 114, 313, BLACK); //填充区域
        LCD_ShowxNum(86, 111, num, 3, 16, 0x80); //显示任务执行次数
        vTaskDelay(1000); //延时 1s，也就是 1000 个时钟节拍
    }
}

