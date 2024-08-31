/**
 * @file watch_dog_timer.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-03-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __WATCH_TIMER_H__
#define __WATCH_TIMER_H__
#include "board_config.h"
#include "sys.h"

/****************************************************************************
 * MACRO
 ****************************************************************************/
//WWDT
// #if IWATCH_DOG_TEST_ENABLE || WWATCH_DOT_TEST_ENABLE
#define WWDT_PRIEMPTION_PRIORITY    2
#define WWDT_SUB_PRIORITY           3
#define WWDT_NVIC_GROUP             2
// #endif
#if WWATCH_DOG_ENABLE == 1
#define WWDG_CNT                    0x7F // T[6:0]:01111111; 保存WWDG计数器的设置值,默认为最大.递减计数器起始值
#define WWDG_UPPER_CNT              0x5F // 上部分触发时间点; 合理喂狗时间:0x5F - 0x40
#define WWDG_CLK_8_PRESCALLER       3 // 00000011:8 分频
#endif
// 喂狗时间时间
// 最小喂狗时间： (T[6:0] - W[6:0]) * Tper
// 最小喂狗时间：(T[6:0] - 0x40) * Tper
// 上边界:0x5F * (4096 * 2 ^ WDGTB)/fpclk1 = 86.47ms
// 下边界:0x40 * (4096 * 2 ^ WDGTB)/fpclk1 = 


void IWDG_Init(u8 prer, u16 rlr);
void WWDG_Init(u8 tr,u8 wr,u8 fprer);
void WWDG_Set_Counter(u8 cnt);
void IWDG_Feed(void);

#endif 

