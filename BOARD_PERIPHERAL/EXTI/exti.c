/**
 * @file exti.c
 * @author your name (you@domain.com)
 * @brief external interrupt
 * @version 0.1
 * @date 2022-02-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "exti.h"
#include "led.h"
#include "key_stm.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "bsp_config.h"
#if DLPS_TEST_ENABLE
#include "dlps.h"
#endif
#if DMA_TEST_ENABLE
#include "dma_stm.h"
#endif
#if MOUSE_ENABLE
#include "ps2.h"
#endif



// function declaration
void EXTI0_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);

/**
 * @brief external interrupt intialization
 * 
 */
// void EXTI_Init(void)
// {

// }

/**
 * @brief intialize the key interrupt
 */
void EXTI_Init()
{
    // key GPIO init
    // KEY_Init();

    // configure NVIC
    GPIO_NVIC_Config(KEY0_GPIO, KEY0_PORT, FTIR);   //rising trigger
    MY_NVIC_Init(KEY0_PRIEMPTION_PRIORITY, KEY0_SUB_PRIORITY, EXTI9_5_IRQn, KEY0_NVIC_GROUP);

    GPIO_NVIC_Config(KEY1_GPIO, KEY1_PORT, FTIR);   //falling trigger
    MY_NVIC_Init(KEY1_PRIEMPTION_PRIORITY, KEY1_SUB_PRIORITY, EXTI15_10_IRQn, KEY1_NVIC_GROUP);

#ifndef DLPS_ENABLE
    GPIO_NVIC_Config(WKUP_GPIO, WKUP_PORT, RTIR);
    MY_NVIC_Init(WKUP_PRIEMPTION_PRIORITY, WKUP_SUB_PRIORITY, EXTI0_IRQn, WKUP_NVIC_GROUP);
#endif // DLPS_ENABLE

    return;
}

/**
 * @brief interrupt line 0 function
 * 
 */
void EXTI0_IRQHandler(void)
{
    // 中断中延时, 测量脉冲宽度的时候会出现异常
    EXTI->PR = 1 << 0; // 清除中断标志
    // delay_ms(10);               // xiao dou 
    if(WK_UP == 1){             // PA0 - wake up key
#if INT_KEY_TEST_ENABLE
        printf("WKUP pressed\n");
        LED0 = !LED0;
        LED1 = !LED1;
#endif
    
#if DLPS_TEST_ENABLE
        // 判断 WKUP 是否按下超过 3s, 超过的话进入 standby mode
        // EXTI->PR = 1 << 0; // 清除中断标志
        if(Check_WKUP()){
            Sys_Enter_Standby();
        }
#endif
    }

    return;
}

/**
 * @brief GPIO 9 - 5 interrupt routine
 * 
 */
void EXTI9_5_IRQHandler(void)
{
    delay_ms(10);

    if(KEY0 == 0){
#if INT_KEY_TEST_ENABLE
        printf("key 0 pressed\n", __func__);
        LED0 = !LED0;
#endif

    // fedd watch dog
#if IWATCH_DOG_TEST_ENABLE
        printf("feed indepedent watch dog timer.\r\n");
        IWDG_Feed();
#elif WWATCH_DOT_TEST_ENABLE
        // printf("feed window watch dog timer.\r\n");
        // WWDG_Set_Counter();
#endif

#if DMA_TEST_ENABLE
        // 开始一次 DMA 传输
        // 等待 DMA 传输完成, 此时我们可以做一些其他事情(不需要 CPU 参与)
        // 实际应用中, DMA 传输期间可以执行另外的任务
        USART1->CR3 |= 1 << 7; // 使能 UART1 的 DMA TX
        DMA_Enable(DMA1_Channel4);
#endif
    }

    // clear interrupt flag
    EXTI->PR = (1 << 5);
}

/**
 * @brief 15 - 10 interrupt routine
 * 
 */
void EXTI15_10_IRQHandler(void)
{
    delay_ms(10);
    if(KEY1 == 0){
#if INT_KEY_TEST_ENABLE
        printf("key 1 pressed\n", __func__);
        LED1 = !LED1;
#endif

#if WATCH_DOG_TEST_ENABLE
        printf("feed watch dog timer.\r\n");
        IWDG_Feed();    // fedd watch dog
#endif

#if MOUSE_ENABLE
    static u8 tempdata = 0;
    static u8 parity = 0; // 用于奇偶校验

    if(EXTI->PR & (1 << 15)){ //中断15产生了相应的中断
        EXTI->PR = 1 << 15;  //清除LINE15上的中断标志位
        if(BIT_Count == 0){
            parity = 0;
            tempdata = 0;
        }

        BIT_Count++;    
        if(BIT_Count > 1 && BIT_Count < 10){ //这里获得数据  
            tempdata >>= 1;
            if(PS2_SDA){
                tempdata |= 0x80;
                parity++;//记录1的个数
            }
        }
        else if(BIT_Count == 10){ //得到校验位
            if(PS2_SDA)
                parity |= 0x80;//校验位为1
        }	

        if(BIT_Count == 11) {//接收到1个字节的数据了
             BIT_Count = parity & 0x7f;//取得1的个数	  
            if(((BIT_Count % 2 == 0) && (parity & 0x80)) || ((BIT_Count % 2 == 1) && (parity & 0x80) == 0))//奇偶校验OK
            {					    
                //PS2_Status|=1<<7;//标记得到数据	   
                BIT_Count = PS2_Status & 0x0f;		
                PS2_DATA_BUF[BIT_Count] = tempdata;//保存数据
                if(BIT_Count < 15)
                    PS2_Status++;    //数据长度加1
                BIT_Count = PS2_Status & 0x30;	     //得到模式	

                switch(BIT_Count){
                case CMDMODE://命令模式下,每收到一个字节都会产生接收完成
                    PS2_Dis_Data_Report();//禁止数据传输
                    PS2_Status |= 1 << 7; //标记得到数据
                break;
                case KEYBOARD:
                break;
                case MOUSE:
                    if(MOUSE_ID == 0){ //标准鼠标,3个字节
                        if((PS2_Status & 0x0f) == 3){
                            PS2_Status |= 1 << 7;//标记得到数据
                            PS2_Dis_Data_Report();//禁止数据传输
                        }
                    }
                    else if(MOUSE_ID == 3){ //扩展鼠标,4个字节
                        if((PS2_Status & 0x0f) == 4){
                            PS2_Status |= 1 << 7;//标记得到数据
                            PS2_Dis_Data_Report();//禁止数据传输
                        }
                    }	 
                break;
                }		   		 
            }
            else{ // 校验失败
                PS2_Status |= 1 << 6;//标记校验错误
                PS2_Status &= 0xf0;//清除接收数据计数器
            }
            BIT_Count = 0;
        } 	 	  
    }
#endif

    }

    EXTI->PR = 1 << 15;
}

