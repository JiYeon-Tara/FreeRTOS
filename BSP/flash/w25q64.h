#ifndef __W25Q64_H__
#define __W25Q64_H__

#include "sys.h"

//W25X系列/Q系列芯片列表	   
//W25Q80 ID  0XEF13
//W25Q16 ID  0XEF14
//W25Q32 ID  0XEF15
//W25Q64 ID  0XEF16	
#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16

extern u16 SPI_FLASH_TYPE;		//定义我们使用的flash芯片型号

void SPI_Flash_Init(void);
u16  SPI_Flash_ReadID(void); //读取FLASH ID
int SPI_Flash_ReadUniqueID(u8 *ptr, u8 *len);
u8	 SPI_Flash_ReadSR(void); //读取状态寄存器 
void SPI_FLASH_Write_SR(u8 sr); //写状态寄存器
void SPI_FLASH_Write_Enable(void); //写使能 
void SPI_FLASH_Write_Disable(void); //写保护
void SPI_Flash_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead); //读取flash
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite); //写入flash
void SPI_Flash_Erase_Chip(void); //整片擦除
void SPI_Flash_Erase_Sector(u32 Dst_Addr);//扇区擦除
void SPI_Flash_Wait_Busy(void); //等待空闲
void SPI_Flash_PowerDown(void); //进入掉电模式
void SPI_Flash_WAKEUP(void); //唤醒

void flash_read_write_test(void);

#endif

