 /**
 * @file spi_stm.c
 * @author your name (you@domain.com)
 * @brief 硬件 SPI 控制器
 * @version 0.1
 * @date 2022-12-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
 #include "spi_stm.h"
 
// 对应的 GPIO
// 全部都接 45K 电阻上拉
// SPI1 CS - PA2
// SPI1 SCK - PA5
// SPI1 MISO - PA6
// SPI1 MOSI - PA7
 
// SPI initialization
//以下是SPI模块的初始化代码，配置成主机模式，访问SD Card/W25Q64/24L01, 上面挂了很多外设, 通过 CS 控制




/**
 * @brief SPI口初始化
 *        这里针是对SPI1的初始化
 * 
 */
void SPI1_Init(void)
{
    RCC->APB2ENR |= 1 << 2;       // PORTA时钟使能 	 
    RCC->APB2ENR |= 1 << 12;      // SPI1时钟使能 
           
    //这里只针对SPI口初始化
    GPIOA->CRL &= 0X000FFFFF; 
    GPIOA->CRL|= 0XBBB00000;    // PA5.6.7复用 	    
    GPIOA->ODR |= 0X7 << 5;     // PA5.6.7上拉
        
    SPI1->CR1 |= 0 << 10;       // 全双工模式(发送和接收)
    SPI1->CR1 |= 1 << 9;        // 软件nss管理
    SPI1->CR1 |= 1 << 8;        // 

    SPI1->CR1 |= 1 << 2;        // SPI主机
    SPI1->CR1 |= 0 << 11;       // 8bit数据格式	
    SPI1->CR1 |= 1 << 1;        // 空闲模式下SCK为1 CPOL=1
    SPI1->CR1 |= 1 << 0;        // 数据采样从第二个时间边沿开始,CPHA=1  
    SPI1->CR1 |= 7 << 3;        // Fsck=Fcpu/256, 波特率控制(正在通信是不可以修改该位)
    SPI1->CR1 |= 0 << 7;        // MSBfirst
    SPI1->CR1 |= 1 << 6;        // SPI设备使能
    SPI1_ReadWriteByte(0xff);   // 启动传输(主要作用：维持MOSI为高)， 写 0xFF 什么作用?
    printf("SPI1 init\r\n");

    return;
}   


//
//SpeedSet:
//
//
/**
 * @brief SPI1 速度设置函数
 * 		  SPI速度=fAPB2/2^(SpeedSet+1), APB2时钟一般为72Mhz
 * 
 * @param SpeedSet 0~7
 */
void SPI1_SetSpeed(u8 SpeedSet)
{
    SpeedSet &= 0X07;			//限制范围0-7
    SPI1->CR1 &= 0XFFC7; 		// 1100 0111, clear bit
    SPI1->CR1 |= SpeedSet << 3;	//设置SPI1速度  
    SPI1->CR1 |= 1 << 6; 		//SPI设备使能 
} 

/**
 * @brief SPI1 读写一个字节
 * 		  为什么没有用到 SPI1_Handler 中断???
 *        写入 0xFF 就是读取吗? 为什么很多地方都用到了这个????, 
 *        写入 0xFF 然后读取一个字节
 * 
 * @param TxData 要写入的字节
 * @return u8 读取到的字节
 */
u8 SPI1_ReadWriteByte(u8 TxData)
{
    u16 retry = 0;				 
    while((SPI1->SR & (1 << 1)) == 0)//等待发送区空	
    {
        retry++;
        if(retry > 0XFFFE)
            return 0;
    }
    SPI1->DR = TxData;	 	  //发送一个byte

    retry = 0;
    while((SPI1->SR & 1 << 0) == 0) //等待接收完一个byte  
    {
        retry++;
        if(retry > 0XFFFE)
            return 0;
    }
                            
    return SPI1->DR;          //返回收到的数据				    
}


