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
// #define KEY_TEST_ENABLE         1
#define LOOP_KEY_ENABLE         1 // 默认模式
#define LOOP_KEY_TEST_ENABLE    0
#define INT_KEY_ENABLE          1 // 中断按键
#define INT_KEY_TEST_ENABLE     1

// OLED & LCD
#define OLED_SCREEN_ENABLE      0
#define OLED_SCREEN_TEST_ENABLE 0

#define LCD_SCREEN_ENABLE       1
#define LCD_SCREEN_TEST_ENABLE  0

// EEPROM
#define EEPROM_ENABLE           1
#define EEPROM_TEST_ENABLE      1

// TODO:
// spi1 的分时复用
// external flash ROM
#define FLASH_ENABLE            0
#define FLASH_TEST_ENABLE       0

// TP
#define TP_ENABLE               1
#define TP_TEST_ENABLE          1

// remote control
#define REMOTE_CONTROL_ENABLE           0
#define REMOTE_CONTROL_TEST_ENABLE      0

// temperature
#define TEMP_ENABLE             0
#define TEMP_TEST_ENABLE        0

// mouse
#define MOUSE_ENABLE            0
#define MOUSE_TEST_ENABLE       0

// sdcard
#define SD_CARD_ENABLE          1
#define SD_CARD_TEST_ENABLE     0


#endif

