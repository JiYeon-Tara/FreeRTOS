/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "sdcard.h""
#include "w25q64.h"
#include "malloc.h"		
// #include "stm_flash.h"


#define SD_CARD	 0  // SD卡, 卷标为 0
#define EX_FLASH 1	// 外部 flash 卷标为 1

#define FLASH_SECTOR_SIZE 	512

// 对于 W25Q64 
//ǰ4.8M�ֽڸ�fatfs��,4.8M�ֽں�~4.8M+100K���û���,4.9M�Ժ�,���ڴ���ֿ�,�ֿ�ռ��3.09M.	
// 前面 4.8M 给 FATFS 使用, 
// 4.8M - (4.8M+100K) 给用户使用
// 4.9M 以后用于存放字库, 字库占用 3.09M
u16	    FLASH_SECTOR_COUNT = 9832;	// 4.8M byte, 默认为 W25Q64
#define FLASH_BLOCK_SIZE   	8     	// 每个 block 有 8 个扇区(sector)


/**
 * @brief 初始化磁盘驱动器
 * 
 * @param[in] pdrv
 * @return 0:init success
 */
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
	u8 res = 0;	
    
	switch(pdrv){
		case SD_CARD://SDcard
			res = SD_Initialize();//SD_Initialize() 
		 	if(res) {// STM32 SPI 的 bug, SD 卡操作失败的时候, 如果不执行下面的操作, 会导致 SPI 读写异常
				SD_SPI_SpeedLow();
				SD_SPI_ReadWriteByte(0xff);//�ṩ�����8��ʱ��
				SD_SPI_SpeedHigh();
			}
  			break;
		case EX_FLASH: // external flash
			SPI_Flash_Init();
			if(SPI_FLASH_TYPE == W25Q64)
				FLASH_SECTOR_COUNT = 9832; // W25Q64
			else 
				FLASH_SECTOR_COUNT = 0; // other
 			break;
		default:
			res = 1; 
	}		 
	if(res)
		return STA_NOINIT;
	// else 
	
	return 0; //init success
}  


/**
 * @brief 获取磁盘驱动器状态
 * 
 * @return
 */
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
	return 0;
} 

/**
 * @brief 读扇区
 * 
 * @param[in]  pdrv 磁盘编号 0 - 9
 * @param[out] buff 接收缓冲区
 * @param[in]  sector 扇区地址
 * @param[in]  count 要读取的扇区书数目
 * @return 0:success; other:fail
 */
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
)
{
	u8 res = 0;
 
    if (!count)
		return RES_PARERR;
 
	switch(pdrv)
	{
		case SD_CARD: //SDcard
			res = SD_ReadDisk(buff, sector, count);	 
		 	if(res) {// STM32 SPI 的 bug, SD 卡操作失败的时候, 如果不执行下面的操作, 会导致 SPI 读写异常
				SD_SPI_SpeedLow();
				SD_SPI_ReadWriteByte(0xff); // 提供额外 8 个时钟
				SD_SPI_SpeedHigh();
			}
			break;
		case EX_FLASH://�ⲿflash
			for(; count > 0; count--){
				SPI_Flash_Read(buff, sector * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
				sector++;
				buff += FLASH_SECTOR_SIZE;
			}
			res = 0;
			break;
		default:
			res = 1; 
	}

	// 将 SPI_SD_Driver 的返回值转换为 ff.c 的返回值
    if(res == 0x00)
		return RES_OK;	 
    else 
		return RES_ERROR;	   
}

#if _USE_WRITE
/**
 * @brief 写扇区
 * 
 * @param[in] pdrv 磁盘编号 0 - 9
 * @param[in] buff 输入地址
 * @param[in] sector
 * @param[in] count
 * @return
 */
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
	u8 res = 0;
  
    if (!count)
		return RES_PARERR;

	switch(pdrv)
	{
		case SD_CARD: // SD card
			res = SD_WriteDisk((u8*)buff, sector, count);
			break;
		case EX_FLASH: // external flash
			for(; count > 0; count--){									    
				SPI_Flash_Write((u8*)buff, sector * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
				sector++;
				buff += FLASH_SECTOR_SIZE;
			}
			res = 0;
			break;
		default:
			res = 1; 
	}

	// 将 SPI_SD_Driver 的返回值转换为 ff.c 的返回值
    if(res == 0x00)
		return RES_OK;	 
    else 
		return RES_ERROR;	
}
#endif


/**
 * @brief 其他参数获得
 * 
 * @param[in] pdrv 磁盘编号
 * @param[in] cmd 控制代码
 * @param[in] buff 数据缓冲区
 * @return 
 */
#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;	
					  			     
	if(pdrv == SD_CARD){
	    switch(cmd){
		    case CTRL_SYNC:
				SD_CS = 0;
		        if(SD_WaitReady() == 0)
					res = RES_OK; 
		        else 
					res = RES_ERROR;	  
				SD_CS = 1;
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(WORD*)buff = 512;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(WORD*)buff = 8;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
				// get sector count 一直 = 0， 文件系统会不会出现问题??????
		        *(DWORD*)buff = SD_GetSectorCount();
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}
	else if(pdrv==EX_FLASH) {
	    switch(cmd) {
		    case CTRL_SYNC:
				res = RES_OK; 
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(WORD*)buff = FLASH_SECTOR_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(WORD*)buff = FLASH_BLOCK_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = FLASH_SECTOR_COUNT;
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}
	else 
		res = RES_ERROR; // 其他盘符驱动不支持
    return res;
}
#endif

// 获得时间
//User defined function to give a current time to fatfs module */
// 31-25: Year(0-127 org.1980), 
// 24-21: Month(1-12), 
// 20-16: Day(1-31) */                                                                                                                                                                                                                                          
// 15-11: Hour(0-23), 
// 10-5: Minute(0-59), 
// 4-0: Second(0-29 *2) */                                                                                                                                                                                                                                                
DWORD get_fattime (void)
{
	printf("sizeof(DWORD) = %d\r\n", sizeof(DWORD)); 
	return 0;
}

// mallocf
void *ff_memalloc (UINT size)			
{
	return (void*)mymalloc(size);
}

// free
void ff_memfree (void* mf)		 
{
	myfree(mf);
}
