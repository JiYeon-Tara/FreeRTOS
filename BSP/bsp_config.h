/**
 * @file bsp_config.h
 * @author your name (you@domain.com)
 * @brief 对不同的扩展外设进行使能控制
 * @version 0.1
 * @date 2022-08-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __BSP_CONFIG_H__
#define __BSP_CONFIG_H__


/** switch **/
// LED
#define LED_ENABLE              1
#define LED_TEST_ENBLE          0

// KEY
#define KEY_ENABLE              1
#define LOOP_KEY_ENABLE         0
#define INT_KEY_ENABLE          1

// UART
#define USART1_ENABLE           1
#define UART1_TEST_ENABLE       0

// watch dog
#define WATCH_DOG_TEST_ENABLE   1

#endif

