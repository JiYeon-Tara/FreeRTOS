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
#include "board.h"

void TIM3_Int_Init(u16 arr, u16 psc);

#endif

