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
#include <stdio.h>
#include <stdlib.h>

#include "board_config.h"
#include "bsp_config.h"
#include "LCD.h"
#include "rtc.h"
#include "adc_stm.h"
#include "dac_stm.h"
#include "dma_stm.h"

//
void bsp_init();
void led_test();
void key_test();
void usart_test();
void watch_dog_test();
void dlps_test();
void ADC_test();
void dac_test();

// third party test
void at_cmd_test();
void oled_screen_test();
void lcd_screen_test();

#endif

