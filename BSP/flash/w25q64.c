/**
 * @file w25q64.c
 * @author your name (you@domain.com)
 * @brief 片外扩展 flash, 8M byte
 * 		  可以在 flash 的基础上实现各种存储机制, 比如 KV, NV...
 *
 * @version 0.1
 * @date 2022-12-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "w25q64.h"
#include "spi_stm.h"
#include "usart.h"
#include "ulog.h"

// falsh 使用 SPI 接口
// NRF_CS, SD_card, flash 等硬件都是通过 SPI1 来通信的, 所以在使用的时候需要"时分复用"
// 通过片选(CS)控制 PA2.PA3.PA4, 拉低表示选中该器件

// SPI1 CS - PA2
// SPI1 SCK - PA5
// SPI1 MISO - PA6
// SPI1 MOSI - PA7

// flash 指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 //write status register1
#define W25X_ReadStatusReg2		0x35 //write status register2
#define W25X_WriteStatusReg		0x01 // write status register1 & 2
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B
#define W25X_HighPerformance    0xA3
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB
#define W25X_ReadUniqueID	    0x4B
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

// flash 状态寄存器(Status Register1)标志位
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

u16 SPI_FLASH_TYPE = W25Q64;//默认就是25Q64
static u8 SPI_FLASH_BUF[4096]; // 4K 的 flash 写缓冲区

u16 SPI_Flash_ReadID2(void);

/**
 * @brief 初始化SPI FLASH的IO口 Flash_W25Q64_Init
 * 
 */
void SPI_Flash_Init(void)
{
    u16 id;

    RCC->APB2ENR |= 1 << 2; // PORTA时钟使能  
    // TODO:
    //这里修改为仅初始化 PA2
    GPIOA->CRL &= 0XFFF000FF; 
    GPIOA->CRL |= 0X00033300; // PA2.3.4 推挽, PA3, PA4 为 NRF/SD card 的片选信号
    GPIOA->ODR |= 0X7 << 2; //PA2.3.4上拉

    SPI1_Init(); // 初始化SPI1
    // SPI1_ReadWriteByte(0xff); // 启动传输(主要作用：维持MOSI为高)， 写 0xFF 什么作用?
    SPI1_SetSpeed(SPI_SPEED_4);	// 设置为18M时钟,高速模式

#if SPI1_TEST_ENABLE
#if SPI1_INT_ENABLE
    SPI_Flash_ReadID2();
#else
    id = SPI_Flash_ReadID();
    LOG_I("flash ID:%X", id);
#endif
#endif /* SPI1_TEST_ENABLE */

    return;
}

//读取 SPI_FLASH 的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
u8 SPI_Flash_ReadSR(void)   
{
    u8 status = 0;

    SPI_FLASH_CS = 0; //使能器件   
    SPI1_ReadWriteByte(W25X_ReadStatusReg); //发送读取状态寄存器命令    
    status = SPI1_ReadWriteByte(0Xff); //读取一个字节, 写 0xFF 就是读取一个字节
    SPI_FLASH_CS = 1; //取消片选

    return status;   
}

// 写SPI_FLASH状态寄存器
// 只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void SPI_FLASH_Write_SR(u8 sr)   
{
    SPI_FLASH_CS = 0; //使能器件   
    SPI1_ReadWriteByte(W25X_WriteStatusReg); //发送写取状态寄存器命令    
    SPI1_ReadWriteByte(sr); //写入一个字节  
    SPI_FLASH_CS = 1; //取消片选     	      
}

//SPI_FLASH写使能	
//将WEL置位   
void SPI_FLASH_Write_Enable(void)   
{
    SPI_FLASH_CS = 0; //使能器件   
    SPI1_ReadWriteByte(W25X_WriteEnable); //发送写使能  
    SPI_FLASH_CS = 1; //取消片选     	      
}

/**
 * @brief SPI_FLASH写禁止
 * 		 将WEL清零  	
 * 
 */
void SPI_FLASH_Write_Disable(void)   
{
    SPI_FLASH_CS=0; //使能器件   
    SPI1_ReadWriteByte(W25X_WriteDisable);     //发送写禁止指令    
    SPI_FLASH_CS=1; //取消片选     	      
}

/**
 * @brief 读取芯片ID W25X16的ID:0XEF14
 * 
 * @return u16 
 */
u16 SPI_Flash_ReadID(void)
{
    u16 id = 0;	  
    SPI_FLASH_CS = 0; // 片选, 拉低选中
    SPI1_ReadWriteByte(W25X_ManufactDeviceID);	//发送读取ID命令, 这里不需要延迟, 直接写,SPI1_ReadWriteByte() 里面有延迟的   
    SPI1_ReadWriteByte(0x00); // dummy
    SPI1_ReadWriteByte(0x00); // dummy
    SPI1_ReadWriteByte(0x01); // 0x00
    id |= SPI1_ReadWriteByte(0xFF) << 8; // 先读到高字节 MSB First
    id |= SPI1_ReadWriteByte(0xFF);
    SPI_FLASH_CS = 1;

    return id;
}

// SPI 中断读取测试
u16 SPI_Flash_ReadID2(void)
{
    u16 id = 0;	  
    SPI_FLASH_CS = 0; // 片选, 拉低选中
    SPI1_Write_Byte(W25X_ManufactDeviceID);	//发送读取ID命令, 这里不需要延迟, 直接写,SPI1_ReadWriteByte() 里面有延迟的   
    SPI1_Write_Byte(0x00); // dummy
    SPI1_Write_Byte(0x00); // dummy
    SPI1_Write_Byte(0x01); // 0x00
    SPI1_Write_Byte(0xFF); // 先读到高字节 MSB First
    SPI1_Write_Byte(0xFF);
    SPI_FLASH_CS = 1;

    return id;
}

/**
 * @brief Read Unique ID(7) 4Bh dummy dummy dummy dummy (ID63-ID0)
 * 
 * @param[in] ptr 
 * @param[in/out] len 
 * @return int 0-success, -1 fail;
 */
int SPI_Flash_ReadUniqueID(u8 *ptr, u8 *len)
{
    int i;

    if (!ptr || *len < 8)
        return -1;

    SPI_FLASH_CS = 0;
    SPI1_ReadWriteByte(W25X_ReadUniqueID);
    SPI1_ReadWriteByte(0x00); // dummy
    SPI1_ReadWriteByte(0x00); // dummy
    SPI1_ReadWriteByte(0x00); // dummy
    SPI1_ReadWriteByte(0x00); // dummy
    for (i = 0; i < 8; ++i) {
        ptr[i] = SPI1_ReadWriteByte(0xFF);
    }
    *len = 8;

    SPI_FLASH_CS = 1;

    return 0;
}

/**
 * @brief 读取SPI FLASH  
 * 		  在指定地址开始读取指定长度的数据, 发送 24 位地址之后, 程序就可以开始读取数据了,其地址会自动增加的,
 *        不过要注意, 读取的地址范围不能超过 W25Q64 的地址范围
 * 
 * @param pBuffer 数据存储区
 * @param ReadAddr 开始读取的地址(24bit)
 * @param NumByteToRead 要读取的字节数(最大65535)
 */
void SPI_Flash_Read(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)   
{
     u16 i;

    SPI_FLASH_CS = 0; // 使能器件   
    SPI1_ReadWriteByte(W25X_ReadData); // 发送读取命令   
    SPI1_ReadWriteByte((u8)((ReadAddr) >> 16)); // 发送24bit地址, MSB first
    SPI1_ReadWriteByte((u8)((ReadAddr) >> 8));   
    SPI1_ReadWriteByte((u8)ReadAddr);
    for (i = 0; i < NumByteToRead; i++) {//循环读数  
        pBuffer[i] = SPI1_ReadWriteByte(0XFF);
    }
    SPI_FLASH_CS = 1; //取消片选, The instruction is completed by driving /CS high.

    return;  	      
}

/**
 * @brief 11.2.9 Fast Read (0Bh)
 * 
 * @param ReadAddr 
 * @param pBuffer 
 * @param NumByteToRead 
 */
void SPI_Flash_Fast_Read(u32 ReadAddr, u8 *pBuffer, u16 NumByteToRead)
{
    u16 i;
    SPI_FLASH_CS = 0; // 使能器件   
    SPI1_ReadWriteByte(W25X_FastReadData); // 发送读取命令   
    SPI1_ReadWriteByte((u8)((ReadAddr) >> 16)); // 发送24bit地址, MSB first
    SPI1_ReadWriteByte((u8)((ReadAddr) >> 8));   
    SPI1_ReadWriteByte((u8)ReadAddr);
    SPI1_ReadWriteByte(0xFF); // dummy
    for (i = 0; i < NumByteToRead; i++) { //循环读数  
        pBuffer[i] = SPI1_ReadWriteByte(0XFF);
    }
    SPI_FLASH_CS = 1; //取消片选, The instruction is completed by driving /CS high.

    return;
}

/**
 * @brief SPI在一页(0~65535)内写入少于256个字节的数据
 * 	      在指定地址开始写入最大256字节的数据
 *        block 直到写入成功
 * 
 * @param pBuffer 数据存储区
 * @param WriteAddr 开始写入的地址(24bit)
 * @param NumByteToWrite 要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
 */
void SPI_Flash_Write_Page(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
    u16 i;

    if (!pBuffer || NumByteToWrite <= 0)
        return;

    SPI_FLASH_Write_Enable(); // SET WEL

    SPI_FLASH_CS = 0; // 使能器件
    SPI1_ReadWriteByte(W25X_PageProgram); // 发送写页命令
    SPI1_ReadWriteByte((u8)((WriteAddr) >> 16)); //发送24bit地址, MSB
    SPI1_ReadWriteByte((u8)((WriteAddr) >> 8));   
    SPI1_ReadWriteByte((u8)WriteAddr);   
    for (i = 0; i < NumByteToWrite; i++)
        SPI1_ReadWriteByte(pBuffer[i]); //循环写数
    SPI_FLASH_CS = 1; //取消片选

    SPI_Flash_Wait_Busy(); //等待写入结束
}

/**
 * @brief 无检验写SPI FLASH 
 * 	      必须确保所写的地址范围内的数据全部为 0XFF,否则在非 0XFF 处写入的数据将失败!
 * 	      具有自动换页功能 
 * 		  在指定地址开始写入指定长度的数据,但是要确保地址不越界!
 * 
 * @param pBuffer 数据存储区
 * @param WriteAddr 开始写入的地址(24bit)
 * @param NumByteToWrite 要写入的字节数(最大65535)
 */
void SPI_Flash_Write_NoCheck(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)   
{
    u16 pageremainSize; // WriteAddr 地址对应的这个 page 剩余的可写字节数

    pageremainSize = 256 - WriteAddr % 256; //单页剩余的字节数		 	    
    if(NumByteToWrite <= pageremainSize)
        pageremainSize = NumByteToWrite;//不大于256个字节

    while (1) {
        SPI_Flash_Write_Page(pBuffer, WriteAddr, pageremainSize);// SPI_Flash_Write_Page 一次最多写 256 bytes
        if (NumByteToWrite == pageremainSize)
            break;//写入结束了
        else { //NumByteToWrite > pageremainSize
            pBuffer += pageremainSize;
            WriteAddr += pageremainSize;	
            NumByteToWrite -= pageremainSize;			  //减去已经写入了的字节数
            if (NumByteToWrite > 256) //一次可以写入256个字节
                pageremainSize = 256;
            else //剩余不够256个字节了
                pageremainSize = NumByteToWrite;
        }
    }

    return;
}

/**
 * @brief 写SPI FLASH  
 * 		  在指定地址开始写入指定长度的数据
 * 		  该函数带擦除操作!
 * 		  一次要写入一个扇区, W25Q64 最小擦除单位为:1扇区
 * 
 * @param pBuffer 数据存储区
 * @param WriteAddr 开始写入的地址(24bit)，  地址计算:第几个扇区 ix * 4096(sector 4K)
 * 		 			片外 flash 的地址是怎么计算的?
 *                  这个地址和芯片的内存地址没有关系
 * @param NumByteToWrite 要写入的字节数(最大65535)  
 */
void SPI_Flash_Write(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)   
{
    u32 secpos; // sector position
    u16 secoff; // sectior offset
    u16 secremain; // remain sector size
     u16 i;    

    secpos = WriteAddr / 4096;	// 扇区地址  
    secoff = WriteAddr % 4096;	// 在扇区内的偏移
    secremain = 4096 - secoff;	// 当前扇区剩余空间大小(bytes)

    if(NumByteToWrite <= secremain)
        secremain = NumByteToWrite;//不大于4096个字节

    while(1) {
        // flash 的读写单位 ?
        // step1:
        SPI_Flash_Read(SPI_FLASH_BUF, secpos * 4096, 4096); // 读出整个扇区的内容

        // step2: 检查扇区中剩余空间的数据是否是 0xFF(原始数据, 擦除后才是 0xFF, 然后才可以写入, 否则写入无效)
        // SPI_FLASH_BUF[secoff + 0] 到 SPI_FLASH_BUF[secoff + secremain - 1] 都是 0xFF
        for(i = 0; i < secremain; i++)//校验数据
        {
            if(SPI_FLASH_BUF[secoff + i] != 0XFF)
                break;// 需要擦除
        }
    
        // 擦除扇区
        if (i < secremain)//需要擦除
        {
            SPI_Flash_Erase_Sector(secpos);//擦除这个扇区
            for(i = 0; i < secremain; i++) //复制
            {
                SPI_FLASH_BUF[i + secoff] = pBuffer[i];	  
            }
            SPI_Flash_Write_NoCheck(SPI_FLASH_BUF, secpos * 4096, 4096);//写入整个扇区  
        }
        else 
            SPI_Flash_Write_NoCheck(pBuffer, WriteAddr, secremain);//写已经擦除了的,直接写入扇区剩余区间. 		
           
        if(NumByteToWrite == secremain)
            break;//写入结束了
        else {//写入未结束, 当前扇区剩余空间不够写入 NumByteToWrite bytes 数据, 在下一个扇区继续写
            secpos++;	//扇区地址增1
            secoff = 0;	//偏移位置为0 	 
               pBuffer += secremain;  //指针偏移
            WriteAddr += secremain;//写地址偏移	   
               NumByteToWrite -= secremain;				//字节数递减
            if(NumByteToWrite > 4096)
                secremain = 4096;	//下一个扇区还是写不完
            else 
                secremain = NumByteToWrite;			//下一个扇区可以写完了
        }	 
    }

    return;
}

//擦除整个芯片
//整片擦除时间:
//W25X16:25s 
//W25X32:40s 
//W25X64:40s 
//等待时间超长...
void SPI_Flash_Erase_Chip(void)   
{
    SPI_FLASH_Write_Enable(); //SET WEL
    SPI_Flash_Wait_Busy();   
    SPI_FLASH_CS = 0; //使能器件   
    SPI1_ReadWriteByte(W25X_ChipErase); //发送片擦除命令  
    SPI_FLASH_CS = 1; //取消片选

    SPI_Flash_Wait_Busy(); //等待芯片擦除结束
}

/**
 * @brief 擦除一个扇区
 *        擦除一个山区的最少时间:150ms
 * 
 * @param Dst_Addr 扇区地址 0~2047 for W25Q64
 */
void SPI_Flash_Erase_Sector(u32 Dst_Addr)   
{
    Dst_Addr *= W25Q64_SECTOR_SIZE; // 4KByte a sector
    SPI_FLASH_Write_Enable(); //SET WEL 	 
    SPI_Flash_Wait_Busy();

    SPI_FLASH_CS = 0; //使能器件   
    SPI1_ReadWriteByte(W25X_SectorErase); //发送扇区擦除指令 
    SPI1_ReadWriteByte((u8)((Dst_Addr)>>16)); //发送24bit地址    
    SPI1_ReadWriteByte((u8)((Dst_Addr)>>8));   
    SPI1_ReadWriteByte((u8)Dst_Addr);  
    SPI_FLASH_CS = 1; //取消片选

    SPI_Flash_Wait_Busy();   				   //等待擦除完成
}

//等待 flash 从 bush -> 空闲
// 第 0 位 flash busy 位
void SPI_Flash_Wait_Busy(void)   
{
    while((SPI_Flash_ReadSR() & 0x01) == 0x01); // 等待BUSY位清空
}

/**
 * @brief 进入掉电模式
 * The lower power consumption makes the Power-down instruction especially useful for battery powered applications
 * 
 */
void SPI_Flash_PowerDown(void)   
{
    SPI_FLASH_CS = 0; //使能器件   
    SPI1_ReadWriteByte(W25X_PowerDown); //发送掉电命令  
    SPI_FLASH_CS = 1; //取消片选     	      
    delay_us(3); //等待TPD
}

//唤醒
void SPI_Flash_WAKEUP(void)   
{
    SPI_FLASH_CS = 0; //使能器件   
    SPI1_ReadWriteByte(W25X_ReleasePowerDown); // send W25X_PowerDown command 0xAB    
    SPI_FLASH_CS = 1; //取消片选     	      
    delay_us(3); //等待TRES1
}
