#ifndef __I2C_H__
#define __I2C_H__
#include "sys.h"

/*****************
 * MACRO
 *****************/
#define I2C_SUCCESS         0
#define I2C_ERR             1


// IO 方向设置(设置 SDA 为输入/输出模式, I2C 为半双工, 分时收发)
#define SDA_IN()  {GPIOC->CRH&=0XFFFF0FFF; GPIOC->CRH|=8<<12;} // 上拉/下拉输入模式
#define SDA_OUT() {GPIOC->CRH&=0XFFFF0FFF; GPIOC->CRH|=3<<12;} // 输出模式, 最大 50 MHz

// IO 操作函数	 
#define IIC_SCL    PCout(12)    // SCL, 设置 GPIO 输出值
#define IIC_SDA    PCout(11)    // SDA, 设置 GPIO 输出值
#define READ_SDA   PCin(11)     // 输入SDA, 读取 GPIO 输入


// IIC 所有操作函数
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	 


#endif
