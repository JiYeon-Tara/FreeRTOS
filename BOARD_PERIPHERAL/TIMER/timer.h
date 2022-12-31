/**
 * @file timer.h
 * @author your name (you@domain.com)
 * @brief 通用定时器以及高级定时器的使用
 *        定时器的功能比较多, 输入捕获, 输出控制（PWM输出等等）
 * @version 0.1
 * @date 2022-03-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __TIMER_H__
#define __TIMER_H__
#include "sys.h"
#include "board_config.h"

/***********************
 * MACRO
 ***********************/
#define TIM3_PRIEMPTION_PRIORITY    1
#define TIM3_SUB_PRIORITY           3
#define TIM3_NVIC_GROUP             2

#define TIM2_PREEMPTION_PRIORITY    2
#define TIM2_SUB_RPIORITY           0
#define TIM2_NVIC_GROUP             2

#define TIM2_CAPTURE_COMPLETE        (1 << 7)   // 0x80
#define TIM2_CAP_HIGH_LEVEL          (1 << 6)   // 0x40
#define TIM2_OVERRUN_TIMES

/***********************
 * VARIABLES
 ***********************/
extern u8 TIM2_CH1_CAPTURE_STA;    // 输入捕获状态标志
extern u16 TIM2_CH1_CAPTURE_VAL;   // 输入捕获值


/***********************
 * FUNCTIONS
 ***********************/
void TIM3_Int_Init(u16 arr, u16 psc);
void TIM1_PWM_Init(u16 arr, u16 psc);
void TIM1_set_reload_val(uint32_t val);


#endif

