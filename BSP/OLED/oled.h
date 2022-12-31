
#ifndef __OLED_H
#define __OLED_H			  	 
#include "sys.h"
#include "stdlib.h"

/**************
 * MACRO
 **************/
#define OLED_8080       1           // 1:并行8080模式
#define OLED_SPI        0           // 0:4线串行模式

#define OLED_MODE       OLED_SPI    //OLED模式设置
#define OLED_RESET_GPIO_CTR      1  // 通过单独的 GPIO 控制 OLED reset 引脚
#define OLED_CHIP       1106

#if OLED_CHIP == 1106
#include "ssh_1106.h"
#elif OLED_CHIP == 1306
#include "ssd_1306.h"
#endif


//-----------------OLED端口定义---------------- 
#if OLED_MODE == OLED_SPI
/********************************************** 调好之后这部分移到 spi_soft.h 里面去 ***************************/
    // #include "spi_soft.h"
    #define OLED_CS     PCout(9)        // 片选引脚
// #if OLED_RESET_GPIO_CTR
    #define OLED_RST    PBout(14)   // 在MINISTM32上直接接到了STM32的复位脚！
// #endif // OLED_RESET_GPIO_CTR
    #define OLED_DC     PCout(8)        // 命令/数据选择引脚:0,表示命令;1,表示数据;
    #define OLED_SCLK   PBout(0)        // SCLK, D0 SPI 时钟线
    #define OLED_SDIN   PBout(1)        // SDIN, D1 SPI 数据线
#elif OLED_MODE == OLED_8080
    // #include "parallel_8080.h"
    #define OLED_CS     PCout(9)        // 片选引脚
    #define OLED_DC     PCout(8)        // 命令/数据选择引脚:0,表示命令;1,表示数据;
    #define OLED_WR     PCout(7)        //
    #define OLED_RD     PCout(6)        //
    // 8080 并口:PB0~7,作为数据线
    #define DATAOUT(x) GPIOB->ODR=(GPIOB->ODR&0xff00)|(x&0x00FF); //输出
    // 芯片需要区分写入的是命令或者数据  
    #define OLED_CMD  0	//写命令
    #define OLED_DATA 1	//写数据
#else
    #error "warning unknown chip"
#endif // OLED_MODE == OLED_SPI


// 供应商提供的 BSP 库接口
#if OLED_MODE == OLED_8080
    #define OLED_CS_Clr()  GPIO_ResetBits(GPIOD,GPIO_Pin_3)//CS
    #define OLED_CS_Set()  GPIO_SetBits(GPIOD,GPIO_Pin_3)
    #define OLED_RST_Clr() GPIO_ResetBits(GPIOD,GPIO_Pin_4)//RES
    #define OLED_RST_Set() GPIO_SetBits(GPIOD,GPIO_Pin_4)
    #define OLED_DC_Clr() GPIO_ResetBits(GPIOD,GPIO_Pin_5)//DC
    #define OLED_DC_Set() GPIO_SetBits(GPIOD,GPIO_Pin_5)
    //PC0~7,作为数据线
    #define DATAOUT(x)    GPIO_Write(GPIOC,x);//输出  
    #define OLED_WR_Clr() GPIO_ResetBits(GPIOG,GPIO_Pin_14)
    #define OLED_WR_Set() GPIO_SetBits(GPIOG,GPIO_Pin_14)
    #define OLED_RD_Clr() GPIO_ResetBits(GPIOG,GPIO_Pin_13)
    #define OLED_RD_Set() GPIO_SetBits(GPIOG,GPIO_Pin_13)

#elif OLED_MODE == OLED_SPI // SPI, 使用4线串行接口
    #define OLED_CS_Clr()   (OLED_CS = 0)   //CS
    #define OLED_CS_Set()   (OLED_CS = 1)
    #define OLED_RST_Clr()  (OLED_RST = 0)  //RES
    #define OLED_RST_Set()  (OLED_RST = 1)
    #define OLED_DC_Clr()   (OLED_DC = 0)   //DC
    #define OLED_DC_Set()   (OLED_DC = 1)
    #define OLED_SCLK_Clr() (OLED_SCLK = 0) //CLK
    #define OLED_SCLK_Set() (OLED_SCLK = 1)
    #define OLED_SDIN_Clr() (OLED_SDIN = 0) //DIN
    #define OLED_SDIN_Set() (OLED_SDIN = 1)
#else
    #error "unknown oled chip"
#endif // OLED_MODE == OLED_8080
       
#endif
     



