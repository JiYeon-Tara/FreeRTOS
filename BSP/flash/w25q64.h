#ifndef __W25Q64_H__
#define __W25Q64_H__

#include "sys.h"
#include "util.h"

/*********************************************************************************
 * MACRO
 *********************************************************************************/
//W25X系列/Q系列芯片列表
#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0x16EF // 0XEF16

// 4Kbytes 为一个 Sector(扇区)
// 16 个sector 为 1 个 Block
// W25Q64 容量为 64Mb, 8M Byte(8196KB), 共有 128 个 Block, 每个 block 64KByte
// 每个 block 分 16 个 sector(扇区), 每个 sector 4K byte
// W25Q64 的最少擦除单位为一个扇区，也就是每次必须最少擦除 4K 个字节。

// 这样我们需要给 W25Q64 开辟一个至少 4K 的缓存区，
// 这样对 SRAM 要求比较高，要求芯片必须有 4K 以上 SRAM 才能很好的操作。

#define W25Q64_FLASH_SIZE       (8 * 1024 * 1024) // 8MB
#define W25Q64_START_ADDR       (0x00) // 片外 flash 的地址也是从 0x0 开始编址的
#define W25Q65_END_ADDR         (W25Q64_FLASH_SIZE - 1) // 8 Mbytes
#define W25Q64_BLOCK_SIZE       (64 * 1024) // 64KB
#define W25Q64_SECTOR_SIZE      (4 * 1024) // 4KB


extern u16 SPI_FLASH_TYPE;		//定义我们使用的flash芯片型号

void SPI_Flash_Init(void);
u16 SPI_Flash_ReadID(void); //读取FLASH ID
int SPI_Flash_ReadUniqueID(u8 *ptr, u8 *len);
u8 SPI_Flash_ReadSR(void); //读取状态寄存器 
void SPI_FLASH_Write_SR(u8 sr); //写状态寄存器
void SPI_FLASH_Write_Enable(void); //写使能 
void SPI_FLASH_Write_Disable(void); //写保护
void SPI_Flash_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead); //读取flash
void SPI_Flash_Fast_Read(u32 ReadAddr, u8 *pBuffer, u16 NumByteToRead);
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite); //写入flash
void SPI_Flash_Erase_Chip(void); //整片擦除
void SPI_Flash_Erase_Sector(u32 Dst_Addr);//扇区擦除
void SPI_Flash_Wait_Busy(void); //等待空闲
void SPI_Flash_PowerDown(void); //进入掉电模式
void SPI_Flash_WAKEUP(void); //唤醒

void flash_read_write_test(void);

#endif

