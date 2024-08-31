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
 #include "ulog.h"
 #include "util.h"

 
// 对应的 GPIO
// 全部都接 45K 电阻上拉
// SPI1 CS - PA2
// SPI1 SCK - PA5
// SPI1 MISO - PA6
// SPI1 MOSI - PA7
 
// SPI initialization
//以下是SPI模块的初始化代码，配置成主机模式，访问SD Card/W25Q64/24L01, 上面挂了很多外设, 通过 CS 控制
//这几个器件在使用的时候, 必须分时复用
// 怎么实现呢???

// #define	SPI_FLASH_CS PAout(2)  	//选中FLASH	


/**
 * @brief SPI口初始化，SPI 主模式, master mode
 *        这里针是对SPI1的初始化, 初始化 PA5,6,7, 没有初始化 PA2, CS 引脚
 * 
 */
void SPI1_Init(void)
{
    RCC->APB2ENR |= 1 << 2; // PORTA时钟使能 	 
    RCC->APB2ENR |= 1 << 12; // SPI1时钟使能

    //这里只针对SPI口初始化, 没有初始化 CS 引脚
    GPIOA->CRL &= ~(0xFFF << 20); //  PA5.6.7复用 clear, 0X000FFFFF
    GPIOA->CRL |= 0XBBB00000; // PA5.6.7复用推挽输出, 0x0B= 1011b,
    GPIOA->ODR |= 0X7 << 5; // PA5.6.7上拉

    SPI1->CR1 |= 0 << 10; // 全双工模式(发送和接收)
    SPI1->CR1 |= 1 << 9; // 软件nss管理(Slave Select, CS)，通过 SPI_CR1的 SSI 位控制 CS 输出电平
    SPI1->CR1 |= 1 << 8; // 内部从设备选择, 这里直接就使能了???
    SPI1->CR1 |= 1 << 2; // SPI主机
    SPI1->CR1 |= 0 << 11; // 8bit数据格式	
    SPI1->CR1 |= 1 << 1; // 空闲模式下SCK为1 CPOL=1(时钟空闲时为高电平)
    SPI1->CR1 |= 1 << 0; // 从时钟第二个边沿开始数据采样,CPHA=1
    SPI1->CR1 |= 7 << 3; // Fsck=Fcpu/256, 波特率控制(正在通信时不可以修改该位)
    SPI1->CR1 |= 0 << 7; // MSBfirst
    SPI1->CR1 |= 1 << 6; // SPI设备使能

#if SPI1_INT_ENABLE == 1
    // SPI1->CR2 |= (1 << 7); // 发送缓冲区空中断使能 (Tx buffer empty interrupt enable)
    SPI1->CR2 |= (1 << 6); // 接收缓冲区非空中断使能 (RX buffer not empty interrupt enable)
    SPI1->CR2 |= (1 << 5); // 错误中断使能 (Error interrupt enable)
#if SPI1_DMA_ENABLE == 1
    SPI1->CR2 |= (1 << 1); // 发送缓冲区DMA使能 (Tx buffer DMA enable)
    SPI1->CR2 |= (1 << 0); // 接收缓冲区DMA使能 (Rx buffer DMA enable)
#endif
    MY_NVIC_Init(NVIC_PREEMPTION_PRIORITY_HIGH, NVIC_SUBPRIORITY_HIGH, SPI1_IRQn, DEFAULT_NVIC_GROUP);
    LOG_I("interrupt enable");
#endif /* SPI1_INT_ENABLE */
    // SPI1_ReadWriteByte(0xff); // 启动传输(主要作用：维持MOSI为高)， 写 0xFF 什么作用?
    LOG_I("SPI1 init\r\n");

    return;
}

/**
 * @brief SPI1 速度设置函数
 * 		  SPI速度=fAPB2/2^(SpeedSet+1), APB2时钟一般为72Mhz
 * 
 * @param SpeedSet 0~7, SPI->CR1[5:3]
 */
void SPI1_SetSpeed(u8 SpeedSet)
{
    SpeedSet &= 0X07; //限制范围0-7
    SPI1->CR1 &= 0XFFC7; // bit[5:3], 1100 0111, clear bit
    SPI1->CR1 |= SpeedSet << 3;	//设置SPI1速度(baudrate)
    SPI1->CR1 |= 1 << 6; //SPI设备使能 
}

// https://blog.csdn.net/m0_69658652/article/details/127240189
//SPI总线上的主机必须在通信开始时配置并生成相应的时钟信号，在每个SPI时钟信号内都会发生全双工数据传输。
// 主机在MOSI线发送一位数据，从机读取一位；
// 从机在MISO线发送一位数据。主机读取一位;
// 这意味无论是否接受有效数据，必须按照顺序发送一些虚拟数据。
/**
 * @brief SPI1 读写一个字节
 * 		  为什么没有用到 SPI1_Handler 中断???
 *        写入 0xFF 就是读取吗???
 *        为什么很多地方都用到了这个????, 
 *        写入 0xFF 然后读取一个字节
 * 
 * @desc 没有开启 SPI 中断的时候, 这个接口可以正常使用, 
 *       开启后就不行了
 * 
 * @param TxData 要写入的字节
 * @return u8 读取到的字节
 */
u8 SPI1_ReadWriteByte(u8 TxData)
{
    u16 retry = 0;
    while ((SPI1->SR & (1 << 1)) == 0) { //SR[1] 等待发送区空
        retry++;
        if(retry > 0XFFFE)
            return 0;
    }
    SPI1->DR = TxData; //发送一个byte

    retry = 0;
    while ((SPI1->SR & (1 << 0)) == 0) {//SR[0] 等待接收完一个byte
        retry++;
        if(retry > 0XFFFE)
            return 0;
    }
                            
    return SPI1->DR;          //返回收到的数据				    
}

/**
 * @brief SPI 中断方式接收, 
 * TODO:       
 * 中断方式接收对于 flash 操作貌似不实用, 因为 SPI 中断上报的数据没有指定的格式
 * 主机没有办法按照指定的格式区分收到的数据类型,
 * 看一下正式项目 SPI 这块是怎么做的 B226 ???
 * 或者看一下开源项目怎么做的 linux???
 * 
 * @param data 
 */
void SPI1_Write_Byte(u8 data)
{
    u16 retry = 0;

    while ((SPI1->SR & (1 << 1)) == 0) { //SR[1] 等待发送区空
        retry++;
        if(retry > 0XFFFE)
            return;
    }
    SPI1->DR = data;

    return;
}

/**
 * @brief SPI send
 * 
 * @param p_data 
 * @param len 
 */
void SPI1_Write_Data(u8 *p_data, u16 len)
{
    int i;

    if (!p_data)
        return;

    for (i = 0; i < len; ++i) {
        SPI1_Write_Byte(p_data[i]);
    }
}

void SPI1_IRQHandler(void)
{
    u8 data = 0x00;
    LOG_D("SPI1->SR:%X", (u16)SPI1->SR);
    if (SPI1->SR & 0x01) {// RXNE：接收缓冲非空 (Receive buffer not empty)
        data = SPI1->DR;
        LOG_D("Data:%02X", data);
    }
}

//TODO:
//spi slave init


