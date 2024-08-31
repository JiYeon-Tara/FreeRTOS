#ifndef __SPI2_SOFT_H__
#define __SPI2_SOFT_H__


#include "sys.h"

#include "util.h"
#include "board_config.h"
#include "bsp_config.h"


// 软件 GPIO 模拟 SPI
// 电阻屏芯片连接引脚
#define PEN  PCin(1) //PC1  INT
#define DOUT PCin(2) //PC2  MISO
#define TDIN PCout(3) //PC3  MOSI
#define TCLK PCout(0) //PC0  SCLK
#define TCS  PCout(13) //PC13 CS 


void SPI2_Soft_Init(void);
u16 SPI2_Soft_Read_AD(u8 CMD);


#endif

