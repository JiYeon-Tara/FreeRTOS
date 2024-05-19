/**
 * @file 24c02.c
 * @author your name (you@domain.com)
 * @brief EEPROM 24C02 
 *        大小:256bytes
 * 		  可以在 EEPROM 中写入一些标志位
 *
 * @version 0.1
 * @date 2022-09-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "24cxx.h"
#include "usart.h"
#include "i2c_soft.h"    // 包含 i2c 头文件
#include "bsp_config.h"
#include <stdlib.h>
#include <string.h>

// EEPROM 是一种特殊形式的闪存，其应用通常是个人电脑中的电压来擦写和重编程
// EEPROM 与 flash 的区别???
// 可读写单位 byte, falsh 按块进行擦除和写;


// I2C 接口，使用 GPIO 软件模拟 I2C 的方式，而不是使用的硬件 I2C 接口
// PC12 - SCL
// PC11 - SDA


//初始化IIC接口
void AT24CXX_Init(void)
{
    IIC_Init(); // 硬件初始化(配置时钟以及 GPIO)
    printf("EEPROM Init\r\n");

    return;
}

/**
 * @brief 在AT24CXX指定地址读出一个数据
 *        需要复合芯片的I2C读取时序
 * 
 * @param ReadAddr ReadAddr:开始读数的地址  
 * @return u8 读到的数据
 */
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{
    u8 data = 0, ret = 0;	
    //uint8_t ret;	  	    																 
    IIC_Start();  // 每读写一个字节都需要发送 start 信号吗???

    // 为了兼容不同型号
    if(EE_TYPE > AT24C16)
    {
        IIC_Send_Byte(0XA0); //发送写命令
        IIC_Wait_Ack();

        IIC_Send_Byte(ReadAddr >> 8);//发送高地址，大端
    }
    else 
        IIC_Send_Byte(0XA0 + ((ReadAddr / 256) << 1));   // 发送器件地址0XA0,写数据

    IIC_Wait_Ack(); 

    IIC_Send_Byte(ReadAddr % 256); // 发送地址低字节
    // IIC_Send_Byte(ReadAddr & 0xFF);
    IIC_Wait_Ack();

    IIC_Start();  	 	   
    IIC_Send_Byte(0XA1);           //进入接收模式	
    IIC_Wait_Ack();	 

    data = IIC_Read_Byte(0);		   
    IIC_Stop(); //产生一个停止条件, 每读写一个字节都需要发送 start 信号吗???

    return data;
}


/**
 * @brief 在AT24CXX指定地址写入一个数据
 * 
 * @param WriteAddr 写入数据的目的地址
 * @param DataToWrite 要写入的数据
 */
void AT24CXX_WriteOneByte(u16 WriteAddr, u8 DataToWrite)
{
    u8 ret = 0;

    IIC_Start();  
    if(EE_TYPE > AT24C16){
        IIC_Send_Byte(0XA0);	    //发送写命令 每读写一个字节都需要发送 start 信号吗???
        IIC_Wait_Ack();
        // if(ret != I2C_SUCCESS){
        // 	printf("I2C read error, does not recv ack, ret:%d\r\n", ret);
        // }
        IIC_Send_Byte(WriteAddr >> 8); //发送高地址	  
    }
    else 
        IIC_Send_Byte(0XA0 + ((WriteAddr / 256) << 1));   //发送器件地址0XA0,写数据 
     
    IIC_Wait_Ack();
    // if(ret != I2C_SUCCESS){
    // 	printf("I2C read error, does not recv ack, ret:%d\r\n", ret);
    // }

    IIC_Send_Byte(WriteAddr % 256);   // 发送低地址 (writeAddr & 0xFF), 这样得到低字节吗?
    IIC_Wait_Ack(); 
    // if(ret != I2C_SUCCESS){
    // 	printf("I2C read error, does not recv ack, ret:%d\r\n", ret);
    // }
                                                 
    IIC_Send_Byte(DataToWrite);     //发送字节							   
    IIC_Wait_Ack();  
    // if(ret != I2C_SUCCESS){
    // 	printf("I2C read error, does not recv ack, ret:%d\r\n", ret);
    // }
           
    IIC_Stop(); // 产生一个停止条件, 每读写一个字节都需要发送 start 信号吗???
    delay_ms(10); // 对于EEPROM器件，每写一次(1 byte)要等待一段时间，否则写失败！	 

    return;
}


/**
 * @brief 在AT24CXX里面的指定地址开始写入长度为Len的数据
 *        该函数用于写入16bit或者32bit的数据.
 * 
 * @param WriteAddr 开始写入的地址  
 * @param DataToWrite 数据数组首地址
 * @param Len 要写入数据的长度2,4
 */
void AT24CXX_WriteLenByte(u16 WriteAddr, u32 DataToWrite, u8 Len)
{  	
    u8 t;
    for(t = 0; t < Len; t++){
        AT24CXX_WriteOneByte(WriteAddr + t, (DataToWrite >> (8 * t)) & 0xff);
    }

    return;
} 


/**
 * @brief 在AT24CXX里面的指定地址开始读出长度为Len的数据
 *        该函数用于读出 16bit 或者 32bit 的数据.
 * 		  仅仅用于读取 1 - 4 bytes
 * 
 * @param[in] ReadAddr 开始读出的地址 
 * @param[in] Len 要读出数据的长度2,4
 * @return u32 数据
 */
u32 AT24CXX_ReadLenByte(u16 ReadAddr, u8 Len)
{  	
    u8 t;
    u32 temp=0;
    for(t = 0; t < Len; t++){
        temp <<= 8;
        temp += AT24CXX_ReadOneByte(ReadAddr + Len - t - 1); 	 				   
    }
    // for(t = 0; t < len; ++t){
    // 	p[t] = AT24CXX_ReadOneByte(ReadAddr + t)
    // }
    return temp;												    
}


/**
 * @brief 检查AT24CXX是否正常
 *        这里用了24XX 的最后一个地址(255)来存储标志字(校验位).
 *        如果用其他24C系列,这个地址要修改
 *        24C02 容量 256bytes, 就是在最后一个字节写 0x55
 * 
 * @return u8 1:检测失败; 0:检测成功
 */
u8 AT24CXX_Check(void)
{
    u8 temp = 0;

    // address:0x00 - 0xFF
    temp = AT24CXX_ReadOneByte(255);// 避免每次开机都写AT24CXX	
    // printf("read data 1:%02X\r\n", temp);		   
    if(temp == 0X55)
        return 0;

    else//排除第一次初始化的情况
    {
        AT24CXX_WriteOneByte(255, 0X55); // 在 EEPROM 的地址 0xFF 处写入 0x55，来表示该芯片正常
        temp = AT24CXX_ReadOneByte(255);	
        printf("read data 2:%d\r\n", temp);		   
  
        if(temp == 0X55)
            return 0;
    }
    return 1;											  
}


/**
 * @brief 在AT24CXX里面的指定地址开始读出指定个数的数据
 * 
 * @param ReadAddr 开始读出的地址 对24c02为0~255
 * @param pBuffer 数据数组首地址
 * @param NumToRead 要读出数据的个数
 */
void AT24CXX_Read(u16 ReadAddr, u8 *pBuffer, u16 NumToRead)
{
    while(NumToRead)
    {
        *pBuffer++ = AT24CXX_ReadOneByte(ReadAddr++);	
        NumToRead--;
    }
}  


/**
 * @brief 在AT24CXX里面的指定地址开始写入指定个数的数据
 * 
 * @param WriteAddr 开始写入的地址 对24c02为0~255
 * @param pBuffer 数据数组首地址
 * @param NumToWrite 要写入数据的个数
 */
void AT24CXX_Write(u16 WriteAddr, u8 *pBuffer, u16 NumToWrite)
{
    while(NumToWrite--)
    {
        AT24CXX_WriteOneByte(WriteAddr, *pBuffer);
        WriteAddr++;
        pBuffer++;
    }
}

typedef struct {
    /* 软件版本 */
    uint8_t bootloader_version[10];
    uint8_t app_version[10];
    uint8_t factory_version[10];
    uint8_t hardware_version[10];

    /* 板上硬件配置 */
    uint8_t have_nfc : 1;
    uint8_t have_gps : 1;
    uint8_t have_bt : 1;
    uint8_t resvered : 5; 
} board_config_t;

/**
 * @brief 
 * 
 */
void AT24CXX_read_write_test(void)
{
    uint16_t write_addr = 0x00;
    board_config_t *p_read;

    board_config_t config = {
        .bootloader_version = "0.0.1",
        .app_version = "0.0.2",
        .factory_version = "0.0.3",
        .hardware_version = "0.0.4",
        .have_nfc = 1,
        .have_gps = 0,
        .have_bt = 1,
    };

    LOG_D("board_config_t size:%d", sizeof(board_config_t));
    AT24CXX_Write(write_addr, &config, sizeof(config));
    LOG_D("write success");
    p_read = (board_config_t*)malloc(sizeof(board_config_t));
    if (!p_read)
        return;
    AT24CXX_Read(write_addr, p_read, sizeof(board_config_t));

    LOG_I("read:");
    LOG_I("bootloader_version:%s", p_read->bootloader_version);
    LOG_I("app_version:%s", p_read->app_version);
    LOG_I("factory_version:%s", p_read->factory_version);
    LOG_I("hardware_version:%s", p_read->hardware_version);
    LOG_I("have_nfc:%d have_gps:%d have_bt:%d", p_read->have_nfc, p_read->have_gps, p_read->have_bt);

    if (memcmp(&config, p_read, sizeof(board_config_t)) == 0)
        LOG_I("eeprom test success");
    else
        LOG_I("eeprom test failed");

    free(p_read);

    return;
}