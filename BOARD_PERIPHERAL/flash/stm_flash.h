#ifndef __STM_FLASH_H__
#define __STM_FLASH_H__
#include "sys.h"
#include "util.h"
#include "ulog.h"
#include "board_config.h"
#include "bsp_config.h"
/*********************************************************************************
 * MACRO
 *********************************************************************************/
//用户根据自己的需要设置
#define STM32_FLASH_SIZE 	    256 // 所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 	    1 // 使能FLASH写入(0，不使能; 1，使能)
//FLASH起始地址
#define STM32_FLASH_BASE        0x08000000 // STM32 FLASH的起始地址
#define STM32_FLASH_SIZE_BYTE   (STM32_FLASH_SIZE * 1024) // unit:Byte
#define STM32_FLASH_END         (STM32_FLASH_BASE + STM32_FLASH_SIZE_BYTE - 1)
//FLASH解锁键值
#define FLASH_KEY1              0X45670123
#define FLASH_KEY2              0XCDEF89AB
#define FLASH_RDPRT_KEY         0X000000A5 // RDPRT 键=0X000000A5


// 地址两个要求:
//1. 2 的整数倍, 半字对齐;
//2. 不能在代码区域范围内, 
//这里暂时划分上一半代码区(0x08000000~0x0801FFFF), 
//下一半作为用户 flash 区域(0x08020000~0x083FFFFF)
#define FLASH_SAVE_ADDR 	    0x08020000 // flash 写入地址, 从第 128K 的地方开始写 
/*********************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************/
void STMFLASH_Unlock(void); //FLASH解锁
void STMFLASH_Lock(void); //FLASH上锁
u8 STMFLASH_GetStatus(void); //获得状态
u8 STMFLASH_WaitDone(u16 time); //等待操作结束
u8 STMFLASH_ErasePage(u32 paddr); //擦除页
u8 STMFLASH_WriteHalfWord(u32 faddr, u16 dat);	//写入半字
u16 STMFLASH_ReadHalfWord(u32 faddr); //读出半字  
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite); //从指定地址开始写入指定长度的数据
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead); //从指定地址开始读出指定长度的数据

//测试写入
void Test_Write(u32 WriteAddr,u16 WriteData);	

#endif

