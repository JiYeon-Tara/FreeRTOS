#include "i2c.h"
#include "delay.h" 	

// 串行总线
// 开始信号, 结束信号, 应答信号
// 软件 I2C:
// 使用 GPIO 模拟 I2C 信号实现, 而不是真正的 I2C
// PC12 - SCL; 
// PC11 - SDA
// 硬件上都有 3.3V 上拉
									 


/**
 * @brief 初始化IIC, 配置 GPIO
 *        PC12 - SCL; PC11 - SDA
 * 
 */
void IIC_Init(void)
{					     
 	RCC->APB2ENR |= 1 << 4;     // 先使能外设IO PORTC时钟 							 
	GPIOC->CRH &= 0XFFF00FFF;   // PC11/12 推挽输出
	GPIOC->CRH |= 0X00033000;	// 
	GPIOC->ODR |= 3 << 11;      // PC11,12 输出高
}

//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     // sda 线输出
	IIC_SDA = 1;	  	  
	IIC_SCL = 1;
	delay_us(4);
 	IIC_SDA = 0;    // START:when CLK is high,DATA change form high to low(falling edge)
	delay_us(4);
	IIC_SCL = 0;    // 钳住I2C总线，准备发送或接收数据 
}

//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL = 0;
	IIC_SDA = 0;
 	delay_us(4); 
	IIC_SCL = 1;	//STOP:when CLK is high DATA change form low to high(rolling edge)
 	delay_us(4); 
	IIC_SDA = 1;//发送I2C总线结束信号 						   	
}
  
/**
 * @brief 等待应答信号到来
 * 
 * @return u8 1，接收应答失败; 0，接收应答成功
 */
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      // SDA设置为输入  
	IIC_SDA = 1;	// 设置为输入还可以写吗?
    delay_us(1);	   
	IIC_SCL = 1;
    delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime > 250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL = 0;	//时钟输出0 

	return 0;  
} 

//产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 0;
	delay_us(2);
	IIC_SCL = 1;
	delay_us(2);
	IIC_SCL = 0;
}

//不产生ACK应答		    
void IIC_NAck(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 1;
	delay_us(2);
	IIC_SCL = 1;
	delay_us(2);
	IIC_SCL = 0;
}	


/**
 * @brief IIC发送一个字节
 *        返回从机有无应答
 * 
 * @param txd 
 * @return * void 1，有应答; 0，无应答
 */
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL = 0;        // 钳住低电平, 拉低时钟开始数据传输

	// 一位一位的写
	// I2C 时序
    for(t = 0; t < 8; t++)
    {
        IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1; 	  
		delay_us(2);   // 对TEA5767这三个延时都是必须的
		IIC_SCL = 1;
		delay_us(2); 
		IIC_SCL = 0;	// CLK 下降沿
		delay_us(2);
    }	 
} 

/**
 * @brief 读1个字节
 *        
 * @param ack ack=1时，发送ACK; ack=0，发送nACK
 * @return * u8 
 */
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i, receive = 0;
	SDA_IN();       // SDA设置为输入
    for(i = 0; i < 8; i++ )
	{
        IIC_SCL = 0; 
        delay_us(2);
		IIC_SCL = 1;	// CLK 上升沿
        receive <<= 1;
        if(READ_SDA)
            receive++;   
		delay_us(1); 
    }

    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK 

    return receive;
}









