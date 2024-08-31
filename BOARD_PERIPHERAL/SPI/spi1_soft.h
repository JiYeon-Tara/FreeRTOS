#ifndef __SPI1_SOFT_H__
#define __SPI1_SOFT_H__

#include "sys.h"
#include "util.h"
#include "board_config.h"
#include "bsp_config.h"

// TODO:
//bit-bang(GPIO仿真SPI)
/********************************************** 调好之后这部分移到 spi_soft.h 里面去 ***************************/
// OLED 屏幕使用
// 四线 SPI, GPIO 模拟 SPI
// #define OLED_CS     PCout(9)        // 片选引脚
// #define OLED_RST    PBout(14)       //在MINISTM32上直接接到了STM32的复位脚！	
// #define OLED_DC     PCout(8)        // 命令/数据选择引脚:0,表示命令;1,表示数据;
// #define OLED_SCLK   PBout(0)        // SCLK, D0 SPI 时钟线
// #define OLED_SDIN   PBout(1)        // SDIN, D1 SPI 数据线

// GPIO 模拟 SPI, 四线 SPI 模式下, 只能向模块写, 不能读
// 单向 SPI，只有 MOSI
#define OLED_CS     PCout(9) // 片选引脚
#define OLED_SCLK   PBout(0) // SCLK, D0 SPI 时钟线
#define OLED_SDIN   PBout(1) // SDIN(MOSI), D1 SPI 数据线

void SPI1_Soft_Init(void);
void SPI1_Soft_Write(u8 byte);

#endif

