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
#include "24cxx.h"
#include "w25q64.h"
#include "remote.h"
#include "ds18b20.h"
#include "stm_flash.h"
#include "service_config.h"
#include "malloc.h"


void bsp_init();
void service_init();

// BSP
void led_test();
void key_test();
void usart_test();
void watch_dog_test();
void dlps_test();
void ADC_test();
void dac_test();
void dma_test();
void inner_flash_test();

// service


// third party test
void at_cmd_test();
void oled_screen_test();
void lcd_screen_test();
void eeprom_test();
void flash_test();
void remote_test();

#endif

