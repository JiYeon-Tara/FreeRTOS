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
#define	SPI_FLASH_CS PAout(2)  	//选中FLASH	
				 
////////////////////////////////////////////////////////////////////////////
 
// flash 指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 


// flash 状态寄存器标志位
//读取 SPI_FLASH 的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
#define W25X_SR_BUSY            0x01
#define W25X_SR_WEL             0x02
#define W25X_SR_BP0             0x04
#define W25X_SR_BP1             0x08
#define W25X_SR_BP2             0x10
#define W25X_SR_TB              0x20
#define W25X_SR_RV              0x40
#define W25X_SR_SPR             0x80




void SPI_Flash_Init(void);
u16  SPI_Flash_ReadID(void);  	    //读取FLASH ID
u8	 SPI_Flash_ReadSR(void);        //读取状态寄存器 
void SPI_FLASH_Write_SR(u8 sr);  	//写状态寄存器
void SPI_FLASH_Write_Enable(void);  //写使能 
void SPI_FLASH_Write_Disable(void);	//写保护
void SPI_Flash_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //读取flash
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);//写入flash
void SPI_Flash_Erase_Chip(void);    	  //整片擦除
void SPI_Flash_Erase_Sector(u32 Dst_Addr);//扇区擦除
void SPI_Flash_Wait_Busy(void);           //等待空闲
void SPI_Flash_PowerDown(void);           //进入掉电模式
void SPI_Flash_WAKEUP(void);			  //唤醒

#endif

