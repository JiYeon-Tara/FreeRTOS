#ifndef __SDCARD_H__
#define __SDCARD_H__
#include "sys.h"
#include "util.h"
/*********************************************************************************
 * MACRO
 *********************************************************************************/
// SD卡类型定义  
#define SD_TYPE_ERR     0X00
#define SD_TYPE_MMC     0X01
#define SD_TYPE_V1      0X02
#define SD_TYPE_V2      0X04
#define SD_TYPE_V2HC    0X06	
//数据写入回应字意义
#define MSD_DATA_OK                0x05
#define MSD_DATA_CRC_ERROR         0x0B
#define MSD_DATA_WRITE_ERROR       0x0D
#define MSD_DATA_OTHER_ERROR       0xFF
//SD卡回应标记字
#define MSD_RESPONSE_SUCCESS       0x00
#define MSD_IN_IDLE_STATE          0x01
#define MSD_ERASE_RESET            0x02
#define MSD_ILLEGAL_COMMAND        0x04
#define MSD_COM_CRC_ERROR          0x08
#define MSD_ERASE_SEQUENCE_ERROR   0x10
#define MSD_ADDRESS_ERROR          0x20
#define MSD_PARAMETER_ERROR        0x40
#define MSD_RESPONSE_FAILURE       0xFF


//这部分应根据具体的连线来修改!
//MiniSTM32开发板使用的是PA3作为SD卡的CS脚.
#define	SD_CS  PAout(3) 	//SD卡片选引脚					    	  
/*********************************************************************************
 * PUBLIC VARIABLES
 *********************************************************************************/
extern u8 SD_Type;			//SD卡的类型
/*********************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************/
u8 SD_SPI_ReadWriteByte(u8 data);
void SD_SPI_SpeedLow(void);
void SD_SPI_SpeedHigh(void);
u8 SD_WaitReady(void); //等待SD卡准备
u8 SD_GetResponse(u8 Response); //获得相应
u8 SD_Initialize(void); //初始化
u8 SD_ReadDisk(u8*buf,u32 sector,u8 cnt); //读块
u8 SD_WriteDisk(u8*buf,u32 sector,u8 cnt); //写块
u32 SD_GetSectorCount(void); //读扇区数
u32 SD_PrintBaseInfo(void);
u8 SD_GetCID(u8 *cid_data); //读SD卡CID
u8 SD_GetCSD(u8 *csd_data); //读SD卡CSD

#endif

