#include "spi2_soft.h"
#include "ulog.h"


/**
 * @brief 初始化 GPIO
 * 
 */
void SPI2_Soft_Init(void)
{
    //注意,时钟使能之后,对GPIO的操作才有效
    //所以上拉之前,必须使能时钟.才能实现真正的上拉输出
    // PC0-3, PC13 initialization
    RCC->APB2ENR |= 1 << 4; //PC时钟使能	   
    RCC->APB2ENR |= 1 << 0; //开启辅助时钟							  
    GPIOC->CRL &= ~(0x0000FFFF << 0); //PC0~3 clear, 0XFFFF0000
    GPIOC->CRL |= 0X00003883;
    GPIOC->CRH &= ~(0x0F << 20); //PC13 clear, 0XFF0FFFFF
    GPIOC->CRH |= (0x30 << 20); //PC13通用推挽输出, 0X00300000
    ////PC0~3 13 全部上拉,0X200f
    GPIOC->ODR |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 13);
    LOG_I("soft spi2 init");
}

/**
 * @brief 向 GPIO 写入 1byte 数据 
 * 
 * @param ch 要写入的数据
 */
static void SPI2_Soft_Write_Byte(u8 ch)    
{
    u8 count = 0;   
    for (count = 0; count < 8; count++)  
    {
        if(ch & 0x80) // 先写高 bit
            TDIN=1;  
        else 
            TDIN=0;   
        ch <<= 1;
        TCLK = 0; 
        delay_us(1);
        TCLK = 1; //上升沿有效	        
    }

    return;	 			    
}

/**
 * @brief SPI读数据 
 *        从触摸屏IC读取adc值
 *        发送一条命令, 然后读取
 * 
 * @param CMD 指令
 * @return u16 读到的数据	   
 */
u16 SPI2_Soft_Read_AD(u8 CMD)
{
    u8 count=0; 	  
    u16 Num=0; 
    TCLK = 0; //先拉低时钟 	 
    TDIN = 0; //拉低数据线
    TCS = 0; //选中触摸屏IC
    SPI2_Soft_Write_Byte(CMD);//发送命令字
    delay_us(6);//ADS7846的转换时间最长为6us
    TCLK = 0;
    delay_us(1);
    TCLK = 1; //给1个时钟，清除BUSY
    delay_us(1);    
    TCLK = 0; 	     	    
    for(count = 0; count < 16; count++)//读出16位数据,只有高12位有效 
    {
        Num <<= 1; 	 
        TCLK = 0;	//下降沿有效  	    	   
        delay_us(1);    
        TCLK = 1;
        if(DOUT) // 当前 bit 为1
            Num++; 		
        else // 当前 bit 为 0
            ; 
    }  	
    Num >>= 4;   	//只有高12位有效.
    TCS = 1;		//释放片选	 
    return (Num);   
} 