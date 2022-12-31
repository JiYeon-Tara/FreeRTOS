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

//WWDT
// #if IWATCH_DOG_TEST_ENABLE || WWATCH_DOT_TEST_ENABLE
#define WWDT_PRIEMPTION_PRIORITY    2
#define WWDT_SUB_PRIORITY           3
#define WWDT_NVIC_GROUP             2
// #endif

/***********************
 * GLOBAL var
 ***********************/
extern u8 WWDG_CNT; 		// 0111 1111; 保存WWDG计数器的设置值,默认为最大.计数器起始值(递减)
extern u8 WWDG_UPPER_CNT;	// 上部分触发时间点; 合理喂狗时间:0x5F - 0x3F
extern u8 SPLIT_FREQ;			// 8 分频


void IWDG_Init(u8 prer, u16 rlr);
void WWDG_Init(u8 tr,u8 wr,u8 fprer);
void WWDG_Set_Counter(u8 cnt);
void IWDG_Feed(void);

#endif 

