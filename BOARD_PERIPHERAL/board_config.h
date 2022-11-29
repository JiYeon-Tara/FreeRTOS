/**
 * @file board_config.h
 * @author your name (you@domain.com)
 * @brief 对芯片内部自带的外设进行使能控制
 * @version 0.1
 * @date 2022-08-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __STM32F103RCT6_MINI_DEVELOPMENT_BOARD__
#define __STM32F103RCT6_MINI_DEVELOPMENT_BOARD__
#include "sys.h"

// XXXX_ENABLE 控制是否对外设初始化
// XXXX_TEST_ENABLE 控制是否进行外设测试


/** switch **/

// usart
#define     UART1_ENABLE            1
#define     UART1_TEST_ENABLE       0

// watch dog
#define     WATCH_DOG_TIMER_ENABLE  1
#define     TIMER_ENABLE            1

// RTC
#define RTC_ENABLE                  1
#define RTC_TEST_ENABLE             0

// DLPS
#define DLPS_ENABLE                 0
#define DLPS_TEST_ENABLE            0

// ADC
#define ADC_ENABLE                  0
#define ADC_TEST_ENABLE             0

// DAC
#define DAC_ENABLE                  0
#define DAC_TEST_ENABLE             0

// DMA
#define DMA_ENABLE                  0
#define DMA_TEST_ENABLE             0

// 内部 flash
// 内部 flash 不可以一直玩, 有可能会误修改代码区, 具体原因还没排查
#define INNER_FLASH_ENABLE          0
#define INNER_FLASH_TEST_ENABLE     0


#endif

