/**
 * @file dlps.c
 * @author your name (you@domain.com)
 * @brief STM32F103RCT6 的低功耗模式
 * @version 0.1
 * @date 2022-09-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "dlps.h"
#include "sys.h"
#include "key_stm.h"
#include "led.h"

// 软件 tickless 模式 配合 MCU 的低功耗模式, 可以实现更低的功耗
// 进入 standby mode: 长按 WK_UP 大于 3s 进入;
// 退出 : 短按 WK_UP(上升沿中断) 唤醒 MCU

/*******************
 * MACRO
 *******************/
#define WKUP_KD                  PAin(0)     // 检查 WKUP 是否被按下
#define WKUP_MAJOR_INT_PRIOR     2
#define WKUP_SUB_INT_PRIOR       2
#define WKUP_INT_GROUP           2

/**
 * @brief 系统进入待机模式
 *        ship mode??
 * 
 */
void Sys_Enter_Standby(void)
{
    printf("enter standby mode\r\n");
    // 这里关闭了串口、GPIO、ADC 、Timer 等所有外设, 也可以不关闭需要的外设(根据实际情况)
    RCC->APB2RSTR |= 0x01FC;    // 复位所有 IO 口
    // RCC->APB2RSTR |= 0xFC;
    Sys_Standby(); // 进入待机模式
}


/**
 * @brief 检测 WK_UP 引脚的信号
 *        烧录完代码后开机默认进入 standby mode, 所以需要长按 WK_UP 3s 开机
 * 
 * @ret: 返回值 1-连按3s以上,开机; 0-不到三秒, 错误触发
 */
uint8_t Check_WKUP(void)
{
    uint8_t pressTime = 0; // 记录按下的时间
    LED0 = 0;

    while(1){
        if(WKUP_KD){
            pressTime++;
            delay_ms(30); // 30 * 100 = 3000ms
            if(pressTime >= 100){ // 按下超过 3s
                LED0 = 0; // 点亮 LED0
                return 1;
            }
        }
        else{ // 按下不足 3s
            LED0 = 1;
            return 0;
        }
    }
}

/**
 * @brief GPIO 初始化
 * 
 */
void WKUP_Init(void)
{
    RCC->APB2ENR |= 1 << 2; // 开使能 PORTA 时钟
    RCC->APB2ENR |= 1 << 0; // 开启辅助时钟
    GPIOA->CRL &= 0xFFFFFFF0; // PA0 设置输入
    GPIOA->CRL |= 0x00000008; // 设置 上拉/下拉输入模式
    GPIO_NVIC_Config(GPIO_A, 0, RTIR); //  rising trigger interrupt

    // 检查是否正常开机, 开机直接进入待机模式，
    // 3s
    if(Check_WKUP()){ // 不是正常开机
        Sys_Standby();
    }

    // 设置 GPIO 的中断优先级
    MY_NVIC_Init(WKUP_MAJOR_INT_PRIOR, WKUP_SUB_INT_PRIOR, EXTI0_IRQn, WKUP_INT_GROUP);

    return;
}

