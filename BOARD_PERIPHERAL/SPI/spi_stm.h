#ifndef __SPI_H__
#define __SPI_H__
#include "sys.h"

#include "board_config.h"
#include "bsp_config.h"
/*********************************************************************************
 * MACRO
 *********************************************************************************/
#define	SPI_FLASH_CS PAout(2)  	//片选信号:选中FLASH	

// SPI总线速度设置,实际上是设置分频系数, 数值越大, 波特率越低
// 参考 《STM32数据手册》 SPI->CR1[5:3]
#define SPI_SPEED_2   		0 // 2 分频
#define SPI_SPEED_4   		1 // 4
#define SPI_SPEED_8   		2 // 8 分频
#define SPI_SPEED_16  		3 // 16 分频
#define SPI_SPEED_32 		4 // 32 分频
#define SPI_SPEED_64 		5 // 64 分频
#define SPI_SPEED_128 		6 // 128 分频
#define SPI_SPEED_256 		7 // 256 分频

#define SPI_SPEED_LOW       SPI_SPEED_256
#define SPI_SPEED_MIDDLE    SPI_SPEED_32
#define SPI_SPEED_HIGH      SPI_SPEED_2
/*********************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************/
void SPI1_Init(void); //初始化SPI口
void SPI1_SetSpeed(u8 SpeedSet); //设置SPI速度   
u8 SPI1_ReadWriteByte(u8 TxData);//SPI总线读写一个字节
void SPI1_Write_Byte(u8 data);
void SPI1_Write_Data(u8 *p_data, u16 len);

#endif

