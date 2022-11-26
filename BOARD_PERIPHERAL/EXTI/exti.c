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
    KEY_Init();

    // configure NVIC
    GPIO_NVIC_Config(KEY0_GPIO, KEY0_PORT, FTIR);   //rising trigger
    GPIO_NVIC_Config(KEY1_GPIO, KEY1_PORT, FTIR);   //falling trigger
    GPIO_NVIC_Config(WKUP_GPIO, WKUP_PORT, RTIR);
    MY_NVIC_Init(KEY0_PRIEMPTION_PRIORITY, KEY0_SUB_PRIORITY, EXTI9_5_IRQn, KEY0_NVIC_GROUP);
    MY_NVIC_Init(KEY1_PRIEMPTION_PRIORITY, KEY1_SUB_PRIORITY, EXTI15_10_IRQn, KEY1_NVIC_GROUP);
    MY_NVIC_Init(WKUP_PRIEMPTION_PRIORITY, WKUP_SUB_PRIORITY, EXTI0_IRQn, WKUP_NVIC_GROUP);

    return;
}

/**
 * @brief interrupt line 0 function
 * 
 */
void EXTI0_IRQHandler(void)
{
#if KEY_TEST_ENABLE
    delay_ms(10); //xiao dou 
    if(WK_UP == 1){ //PA0 - wake up key
        printf("%s\n", __func__);
        LED0 = !LED0;
        LED1 = !LED1;
    }
    EXTI->PR = (1<<0);  // 清除中断
#else if DLPS_TEST_ENABLE
    // 判断 WKUP 是否按下超过 3s
    EXTI->PR = 1 << 0; // 清除中断标志
    if(Check_WKUP()){
        Sys_Enter_Standby();
    }
#endif
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
        printf("%s\n", __func__);
        LED0 = !LED0;
#if WATCH_DOG_TEST_ENABLE
        printf("feed watch dog timer.\r\n");
        IWDG_Feed();    // fedd watch dog
#endif

#if DMA_TEST_ENABLE
        // 开始一次 DMA 传输
        // 等待 DMA 传输完成, 此时我们可以做一些其他事情(不需要 CPU 参与)
        // 实际应用中, DMA 传输期间可以执行另外的任务
        USART1->CR3 |= 1 << 7; // 使能 UART1 的 DMA TX
        DMA_Enable(DMA1_Channel4);
#endif
    }

    EXTI->PR = (1 << 5);    //clear interrupt flag
}

/**
 * @brief 15 - 10 interrupt routine
 * 
 */
void EXTI15_10_IRQHandler(void)
{
    delay_ms(10);
    if(KEY1 == 0){
#if WATCH_DOG_TEST_ENABLE
        printf("feed watch dog timer.\r\n");
        IWDG_Feed();    // fedd watch dog
#endif
        printf("%s\n", __func__);
        LED1 = !LED1;
    }

    EXTI->PR = 1 << 15;
}

