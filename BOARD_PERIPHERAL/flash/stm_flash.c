/**
 * @file stm_flash.c
 * @author your name (you@domain.com)
 * @brief STM32 的内部 flash
 * @version 0.1
 * @date 2022-09-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "stm_flash.h"
#include "usart.h"

// 内部 flash: 大小256 K bytes
// 1 page = 2K bytes
// 包括主存储器, 信息块, 闪存存储器接口寄存器
 

//解锁STM32的FLASH
void STMFLASH_Unlock(void)
{
    FLASH->KEYR = FLASH_KEY1;   // 写入解锁序列, 通用地址直接寻址
    FLASH->KEYR = FLASH_KEY2;
}

//flash上锁
void STMFLASH_Lock(void)
{
  FLASH->CR |= 1 << 7;//上锁
}

//得到FLASH状态
u8 STMFLASH_GetStatus(void)
{	
	u32 res;	
	
	res = FLASH->SR; 
	if(res & (1 << 0))
        return 1;		    //忙
	else if(res & (1 << 2))
        return 2;	//编程错误
	else if(res & (1 << 4))
        return 3;	//写保护错误
	return 0;						//操作完成
}

//等待操作完成
//time:要延时的长短
//返回值:状态. 0成功; 1-忙; 2-编程错误; 0xFF-超时;
u8 STMFLASH_WaitDone(u16 time)
{
	u8 res;
	do
	{
		res = STMFLASH_GetStatus();
		if(res != 1)
            break;//非忙,无需等待了,直接退出.
		delay_us(1);
		time--;
	 }while(time);
	 if(time == 0)
        res = 0xff;//TIMEOUT
	 return res;
}

/**
 * @brief 擦除页
 * 
 * @param paddr 页地址
 * @return u8 执行情况
 */
u8 STMFLASH_ErasePage(u32 paddr)
{
	u8 res=0;
	res = STMFLASH_WaitDone(0X5FFF); //等待上次操作结束,>20ms    
	if(res == 0)
	{ 
		FLASH->CR |= 1<<1;//页擦除
		FLASH->AR = paddr;//设置页地址 
		FLASH->CR |= 1 << 6;//开始擦除		  
		res = STMFLASH_WaitDone(0X5FFF);//等待操作结束,>20ms  
		if(res != 1)//非忙
		{
			FLASH->CR &= ~(1 << 1);//清除页擦除标志.
		}
	}
	return res;
}

/**
 * @brief 在FLASH指定地址写入半字
 *        直接操作内存, 内部 flash 内存统一编址
 * 
 * @param faddr 指定地址(此地址必须为2的倍数!!)
 * @param dat 要写入的数据
 * @return u8 
 */
u8 STMFLASH_WriteHalfWord(u32 faddr, u16 dat)
{
	u8 res;	   	    
	res = STMFLASH_WaitDone(0XFF);	 
	if(res==0)//OK
	{
		FLASH->CR |= 1 << 0;//编程使能
		*(vu16*)faddr = dat;//写入数据
		res = STMFLASH_WaitDone(0XFF);//等待操作完成
		if(res != 1)//操作成功
		{
			FLASH->CR &= ~(1 << 0);//清除PG位.
		}
	} 
	return res;
} 

/**
 * @brief 读取指定地址的半字(16位数据) 
 *        直接访问地址
 * 
 * @param faddr 读地址 
 * @return u16 对应数据.
 */
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}

#if STM32_FLASH_WREN	//如果使能了写   

/**
 * @brief 不检查的写入
 * 
 * @param WriteAddr 起始地址
 * @param pBuffer 数据指针; uint16_t 
 * @param NumToWrite 半字(16位)数, 注意要写入多少个半字
 * @return * void 
 */
void STMFLASH_Write_NoCheck(u32 WriteAddr, u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i = 0; i < NumToWrite; i++)
	{
		STMFLASH_WriteHalfWord(WriteAddr, pBuffer[i]);
	    WriteAddr += 2;//地址增加2.
	}  
} 


#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //字节; 小容量芯片(flash size < 256 bytes), 1 page = 1 K byte
#else 
#define STM_SECTOR_SIZE	2048 // 大容量芯片: 1 page = 2 K bytes
#endif

// 扇区大小
u16 STMFLASH_BUF[STM_SECTOR_SIZE / 2];  //最多是2K字节; 最多一次写入一页?

/**
 * @brief 从指定地址开始写入指定长度的数据
 * 
 * @param WriteAddr 起始地址(此地址必须为2的倍数!!)
 * @param pBuffer 数据指针, uint16_t
 * @param NumToWrite 半字(16位)数(就是要写入的16位数据的个数.)
 */
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   // 扇区地址
	u16 secoff;	   // 扇区内偏移地址(16位字计算)
	u16 secremain; // 扇区内剩余地址(16位字计算)	   
 	u16 i;    
	u32 offaddr;   // 去掉0X08000000后的地址

    // 从 0x08000000 开始的 256K byte
	if(WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
        return;//非法地址

	STMFLASH_Unlock();						//解锁
	offaddr = WriteAddr - STM32_FLASH_BASE;		//实际偏移地址.
	secpos = offaddr / STM_SECTOR_SIZE;			//扇区地址  0~127 for STM32F103RBT6
	secoff = (offaddr % STM_SECTOR_SIZE) / 2;		//在扇区内的偏移(2个字节为基本单位.)
	secremain = STM_SECTOR_SIZE / 2 - secoff;		//扇区剩余空间大小   
	if(NumToWrite <= secremain) //不大于该扇区范围, 该扇区剩余空间够用(单位:半字)
        secremain = NumToWrite;

	while(1) 
	{	
        // step1:读出整个扇区的内容
        // falsh 基地址 + 扇区偏移地址
		STMFLASH_Read(STM32_FLASH_BASE + secpos * STM_SECTOR_SIZE, STMFLASH_BUF, STM_SECTOR_SIZE / 2); //读出整个扇区的内容
    
        // step2:校验数据
		for(i = 0; i < secremain; i++)//校验数据
		{
			if(STMFLASH_BUF[secoff + i] != 0XFFFF)//需要擦除状态才可以写入新数据
                break;
		}

        // stp3:写入数据; 如果没有擦除, 先擦出然后写入;
		if(i < secremain)//需要擦除
		{
			STMFLASH_ErasePage(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE);// 擦除这个扇区(page, sector)
            // memcpy ?
			for(i = 0; i < secremain; i++) //先把数据到提前准备好的容器中
			{
				STMFLASH_BUF[i + secoff] = pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(STM32_FLASH_BASE + secpos * STM_SECTOR_SIZE, STMFLASH_BUF, STM_SECTOR_SIZE / 2);//写入整个扇区  
		}
        else 
            STMFLASH_Write_NoCheck(WriteAddr, pBuffer, secremain);//写已经擦除了的,直接写入扇区剩余区间. 	

		if(NumToWrite == secremain)
            break;//写入结束了
		else//写入未结束
		{
			secpos++;				//扇区地址增1
			secoff = 0;				//偏移位置为0 	 
		   	pBuffer += secremain;  	//指针偏移
			WriteAddr += secremain * 2;	//写地址偏移(16位数据地址,需要*2)	   
		   	NumToWrite -= secremain;	//字节(16位)数递减
			if(NumToWrite > (STM_SECTOR_SIZE / 2))//下一个扇区还是写不完
                secremain = STM_SECTOR_SIZE / 2;
			else 
                secremain = NumToWrite;//下一个扇区可以写完了
		}	 
	}
	STMFLASH_Lock();//上锁
}
#endif


/**
 * @brief 从指定地址开始读出指定长度的数据
 * 
 * @param ReadAddr 起始地址
 * @param pBuffer 数据指针
 * @param NumToRead 半字(16位)数
 */
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i = 0; i < NumToRead; i++)
	{
		pBuffer[i] = STMFLASH_ReadHalfWord(ReadAddr);   //每次读取一个半字(2个字节)
		ReadAddr += 2;//偏移2个字节.	
	}
    return;
}


//////////////////////////////////////////测试用///////////////////////////////////////////
/**
 * @brief 
 * 
 * @param WriteAddr 起始地址
 * @param WriteData 要写入的数据
 */
void Test_Write(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr, &WriteData, 1);//写入一个字 
}
