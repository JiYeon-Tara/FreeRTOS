/**
 * @file test.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __TEST_H__
#define __TEST_H__
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h" 
#include "key_stm.h" 
#include "service_usart.h"
#include "watch_dog_timer.h"
#include "timer.h"
#include "ILI9341.h"
#include "ssd_1306.h"
#include "usmart.h"
#include "exti.h"


void bsp_init();
void led_test();
void key_test();
void usart_test();
void watch_dog_test();

#endif

