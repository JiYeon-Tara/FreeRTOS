/**
 * @file spi_soft.c
 * @author your name (you@domain.com)
 * @brief GPIO 模拟 SPI
 * @version 0.1
 * @date 2022-12-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "spi_soft.h"

/********************************************** 调好之后这部分移到 spi_soft.h 里面去 ***************************/
// OLED 屏幕使用
// 四线 SPI, GPIO 模拟 SPI
// #define OLED_CS     PCout(9)        // 片选引脚
// #define OLED_RST    PBout(14)       //在MINISTM32上直接接到了STM32的复位脚！	
// #define OLED_DC     PCout(8)        // 命令/数据选择引脚:0,表示命令;1,表示数据;
// #define OLED_SCLK   PBout(0)        // SCLK, D0 SPI 时钟线
// #define OLED_SDIN   PBout(1)        // SDIN, D1 SPI 数据线