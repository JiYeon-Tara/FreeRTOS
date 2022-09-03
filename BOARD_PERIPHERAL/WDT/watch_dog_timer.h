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
#if WATCH_DOG_TIMER_ENABLE
#define WWDT_PRIEMPTION_PRIORITY    2
#define WWDT_SUB_PRIORITY           3
#define WWDT_NVIC_GROUP             2
#endif

/**
 * @brief  ���ͨ��һ��Ӳ�����Ź�ʵ�ֶ���������Ź�???????????????????????????????????
 * 
 * @param prer 
 * @param rlr 
 */
void IWDG_Init(u8 prer, u16 rlr);
void WWDG_Init(u8 tr,u8 wr,u8 fprer);
void WWDG_Set_Counter(u8 cnt);
void IWDG_Feed(void);

#endif 

