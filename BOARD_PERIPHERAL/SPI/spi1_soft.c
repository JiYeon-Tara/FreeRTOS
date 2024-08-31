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
#include "spi1_soft.h"
#include "ulog.h"


void SPI1_Soft_Init(void)
{
    GPIOB->CRL &= 0XFFFFFF00; // PB1, PB2 clear
    GPIOB->CRL |= 0XF0000033; // output push pull
    GPIOB->ODR |= (3 << 0);   // 0X03 set output 1

    GPIOC->CRH &= 0XFFFFFF00; // PC8, PC9 clear  
    GPIOC->CRH |= 0X00000033; // output push pull
    GPIOC->ODR |= (3 << 8);     // PC[9:8] output 1
    LOG_I("SPI1_Soft_Init");
}

void SPI1_Soft_Write(u8 byte)
{
    u8 i;

    OLED_CS = 0; // step1:拉低 CS片选(polarity=1)

    for (i = 0; i < 8; i++) { // 从高位到低位依次写入
        // step2: CLK 第一个上升沿读取数据: CPHA=0, 相位
        OLED_SCLK = 0; // 拉低时钟, SPI 仅仅在时钟的边沿通信, 所以这里拉低拉低时钟
        // 先把数据准备好, 然后制造上升沿, 让从机读取
        if(byte & 0x80)
           OLED_SDIN = 1; // 1
        else
           OLED_SDIN = 0; // 0
        OLED_SCLK = 1; // 拉高时钟, 产生上升沿
        byte <<= 1; //MSB first
        //TODO:
        // 这里不用加点延时吗? 不同型号的 CPU 执行速度不一样吧????
        // 这里最好可以用逻辑分析仪看一下波形
    }
    OLED_CS = 1; // 恢复片选

    return;
}
