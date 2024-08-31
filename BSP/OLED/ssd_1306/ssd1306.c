/**
 * @file ssd1306.c
 * @author your name (you@domain.com)
 * @brief BSP, 这个文件起始就是"正点原子"在芯片厂商提供的驱动库的基础上移植了一下, 仅仅是做了"驱动工程师"的活
 *        与供应商提供的库不太一样, 使用了外部显存 GRAM, 避免了读出再写入的情况出现
 *
 * @version 0.1
 * @date 2022-12-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "ssd_1306.h"
#include "stdlib.h"
#include "oledfont.h"  	 
#include "delay.h" 
#include "usart.h"
 
 #if OLED_SCREEN_ENABLE == 1
// ssd1306 control command list, 具体参数格式要看手册
#define SSD1306_SETUP_CONTRAST_RATIO_CMD    0x81
#define SSD1306_OPEN_DISPLAY_CMD            0xAE
#define SSD1306_CLOSE_DISPLAY_CMD           0xAF
#define SSD1306_OPEN_CHARGE_CMD             0x8D // 打开电荷泵
#define SSD1306_SET_PAGE0_ADDR_CMD           0xB0 // 设置第0页页地址
#define SSD1306_SET_PAGE1_ADDR_CMD           0xB1 // 设置第1页页地址
#define SSD1306_SET_PAGE2_ADDR_CMD           0xB2 // 设置第2页页地址
#define SSD1306_SET_PAGE3_ADDR_CMD           0xB3 // 设置第3页页地址
#define SSD1306_SET_PAGE4_ADDR_CMD           0xB4 // 设置第4页页地址
#define SSD1306_SET_PAGE5_ADDR_CMD           0xB5 // 设置第5页页地址
#define SSD1306_SET_PAGE6_ADDR_CMD           0xB6 // 设置第6页页地址
#define SSD1306_SET_PAGE7_ADDR_CMD           0xB7 // 设置第7页页地址
#define SSD1306_SET_START_COLUMN_NUM_LOW_CMD           0x00 // 设置 0x00-0x0F 设置显示时起始列地址的低四位
#define SSD1306_SET_START_COLUMN_NUM_HIGH_CMD           0x10 // 设置 0x10-0x1F 设置显示时起始列地址的低四位

// OLED, chapter 15 还需要再认真看看, 如何切换不同的字库之类的;

// OLED的显存
// 存放格式如下.
// page[0]0 1 2 3 ... 127	
// [1]0 1 2 3 ... 127	
// [2]0 1 2 3 ... 127	
// [3]0 1 2 3 ... 127	
// [4]0 1 2 3 ... 127	
// [5]0 1 2 3 ... 127	
// [6]0 1 2 3 ... 127	
// [7]0 1 2 3 ... 127 		   
u8 OLED_GRAM[128][8]; // 8 * 128 矩阵保存 OLED 点阵数据(128*64)

//
/**
 * @brief 更新显存到LCD
 * 		  refresh screen
 * 
 */
void OLED_Refresh_Gram(void)
{
    u8 i, n;
    for(i = 0; i < 8; i++){
        OLED_WR_Byte (0xb0 + i, OLED_CMD);    //设置页地址0xB0 - 0xB7（0~7）
        OLED_WR_Byte (0x00, OLED_CMD);      //设置显示起始位置—显示起始列低地址低 4 bit
        OLED_WR_Byte (0x10, OLED_CMD);      //设置显示位置—列高地址   
        for(n = 0; n < 128; n++){
            OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA); 
        }
    }   
}

#if OLED_MODE == OLED_8080
/**
 * @brief 向SSD1306写入一个字节。
 * 
 * @param dat 要写入的数据/命令
 * @param cmd 数据/命令标志 0,表示命令;1,表示数据;
 */
void OLED_WR_Byte(u8 dat,u8 cmd)
{
    DATAOUT(dat);	    
    OLED_DC=cmd;
    OLED_CS=0;	   
    OLED_WR=0;	 
    OLED_WR=1;
    OLED_CS=1;	  
    OLED_DC=1;	 
}
#else
/**
 * @brief 向SSD1306写入一个字节。
 *        按照芯片指定的 SPI 操作时序来
 * 
 * @param data 要写入的数据/命令data
 * @param cmd 数据/命令标志 0,表示命令;1,表示数据;
 */
void OLED_WR_Byte(u8 dat, u8 cmd)
{
    u8 i;

    OLED_DC = cmd; // 写命令/数据
    OLED_CS = 0; // 片选
    for(i = 0; i < 8; ++i){
        OLED_SCLK = 0; // SPI 在 CLK 的边沿写入/读取数据
        // SPI 从高位开始写:bit[7] -> bit[6] ->... -> bit[0]
        if(dat & 0x80) // (1 << 8), 最高位
            OLED_SDIN = 1;
        else 
            OLED_SDIN = 0;
        OLED_SCLK = 1;
        dat <<= 1; 
    }
    OLED_CS = 1;		  
    OLED_DC = 1;

    return;   	  
} 
#endif
            
//开启OLED显示    
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC命令
    OLED_WR_Byte(0X14, OLED_CMD);  //DCDC ON
    OLED_WR_Byte(0XAF, OLED_CMD);  //DISPLAY ON
}

//关闭OLED显示     
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
    OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
    OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}	

/**
 * @brief 清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!	 
 * 
 * @return * void 
 */
void OLED_Clear(void)  
{
    u8 i, n;
  
    for(i = 0; i < 8; i++)
        for(n = 0; n < 128; n++)
            OLED_GRAM[n][i] = 0X00;  
    OLED_Refresh_Gram();//更新显示

    return;
}

/**
 * @brief 画点, 计算要写入本地显存中的 bit, 然后更新本地显存 GRAM
 *        没有写入到驱动显存
 * 
 * @param x :0~127
 * @param y 0~63
 * @param t 1 填充; 0,清空, 写GRAM中的某一个位	
 * @return * void 
 */
void OLED_DrawPoint(u8 x, u8 y, u8 t)
{
    u8 pos, bx, temp = 0;

    if(x > 127 || y > 63) //超出范围了.
        return;
    pos = 7 - y / 8;
    bx = y % 8;
    temp = 1 << (7 - bx);
    if(t)
        OLED_GRAM[x][pos] |= temp;
    else 
        OLED_GRAM[x][pos] &= ~temp;

    return;
}

/**
 * @brief 填充/清空矩形区域
 * 
 * @param x1 x1,y1,x2,y2 填充区域的对角坐标
 * @param y1 确保x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63	
 * @param x2 
 * @param y2 
 * @param dot 0,清空;1,填充
 */
void OLED_Fill(u8 x1, u8 y1, u8 x2, u8 y2, u8 dot)  
{
    u8 x, y;  

    for(x=x1;x<=x2;x++)
    {
        for(y=y1;y<=y2;y++)OLED_DrawPoint(x,y,dot);
    }													    
    OLED_Refresh_Gram();//更新显示
}

/**
 * @brief 在指定位置显示一个字符,包括部分字符, 显示很多一个字符 character
 * 
 * @param x :0~127
 * @param y 0~63
 * @param chr 要显示的字符
 * @param size 选择字体 12/16/24
 * @param mode chr 0,反白显示;1,正常显示	
 */
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode)
{
    u8 temp, t, t1;
    u8 y0 = y;
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); //得到字体一个字符对应点阵集所占的字节数
    chr = chr - ' '; //得到偏移后的值
		 
    for(t = 0;t < csize; t++)
    {
        if(size == 12)
            temp = asc2_1206[chr][t]; 	 	//调用1206字体
        else if(size == 16)
            temp = asc2_1608[chr][t];	//调用1608字体
        else if(size == 24)
            temp = asc2_2412[chr][t];	//调用2412字体
        else 
            return;	// 没有的字库

        for(t1 = 0; t1 < 8; t1++)
        {
            if(temp & 0x80)
                OLED_DrawPoint(x, y, mode);
            else 
                OLED_DrawPoint(x, y, !mode);

            temp <<= 1;
            y++;
            if((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }  	 
    }

    return;
}

//m^n函数
u32 mypow(u8 m,u8 n)
{
    u32 result = 1;	 

    while(n--)
        result *= m;  
  
    return result;
}	

//mode:模式	0,填充模式;1,叠加模式
/**
 * @brief 显示2个数字
 * 
 * @param x 起点坐标	 
 * @param y 
 * @param num 数值(0~4294967295);
 * @param len 数字的位数
 * @param size 字体大小
 * @return * void 
 */
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size)
{
    u8 t,temp;
    u8 enshow = 0;
						   
    for(t = 0; t < len; t++)
    {
        temp = (num / mypow(10, len-t-1)) % 10;
        if(enshow == 0 && t < (len - 1))
        {
            if(temp == 0)
            {
                OLED_ShowChar(x + (size / 2) * t, y, ' ', size, 1);
                continue;
            }
            else 
                enshow = 1; 
              
        }
        OLED_ShowChar(x+(size/2)*t,y,temp+'0',size,1); 
    }

    return;
} 

//显示字符串， 显示一个字符串
//x,y:起点坐标  
//size:字体大小 
//*p:字符串起始地址 
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size)
{	
    while((*p <= '~') && (*p >= ' '))//判断是不是非法字符!
    {
        if(x>(128-(size/2))){
            x=0;y+=size;
        }
        if(y>(64-size)){
            y=x=0;OLED_Clear();
        }
        OLED_ShowChar(x,y,*p,size,1);	 
        x+=size/2;
        p++;
    }
}
    
/**
 * @brief 初始化SSD1306	
 * 
 * @return * void 
 */
void OLED_Init(void)
{
    // step1:初始化通讯用到的 GPIO
    RCC->APB2ENR |= 1 << 3; //使能PORTB时钟 
    RCC->APB2ENR |= 1 << 4; //使能PORTC时钟 	  
#if OLED_MODE == OLED_8080 //使用8080并口模式				 
    JTAG_Set(SWD_ENABLE);
    GPIOB->CRL=0X33333333;
    GPIOB->ODR|=0XFFFF;								    	 
 
    GPIOC->CRH&=0XFFFFFF00;
    GPIOC->CRL&=0X00FFFFFF;
    GPIOC->CRH|=0X00000033;
    GPIOC->CRL|=0X33000000;
    GPIOC->ODR|=0X03C0;
#else //使用4线 SPI 串口模式
    GPIOB->CRL &= 0XFFFFFF00; // PB1, PB2clear
    GPIOB->CRL |= 0XF0000033; // output push pull
    GPIOB->ODR |= (3 << 0);   // 0X03 set output 1

#if OLED_RESET_GPIO_CTR
    // 为什么这些管脚输出法只有 1.6V -> 寄存器操作前必须要先清零(不需要的寄存器要复位)
    // GPIOB->CRH &= 0xFF0FFFFF; // PB13 clear
    // GPIOB->CRH |= 0x00300000; // output push pull
    // GPIOB->ODR |= (1 << 13);  // output 1

    GPIOB->CRH &= 0xF0FFFFFF; // PB14 clear
    GPIOB->CRH |= 0x03000000; // output push pull
    GPIOB->ODR |= (1 << 14);  // output 1

    // GPIOB->CRH &= 0x0FFFFFFF; // PB15 clear
    // GPIOB->CRH |= 0x30000000; // output push pull
    // GPIOB->ODR |= (1 << 15);  // output 1
#endif /* OLED_RESET_GPIO_CTR */

    GPIOC->CRH &= 0XFFFFFF00; // PC8, PC9 clear  
    GPIOC->CRH |= 0X00000033; // output push pull
    GPIOC->ODR |= (3 << 8);     // output 1

#endif /* OLED_MODE */

    // step2: ssd1306 启动流程
    OLED_RST = 1; 
    delay_ms(100);  
    OLED_RST = 0;
    delay_ms(100);
    OLED_RST = 1; 
                      
    OLED_WR_Byte(0xAE,OLED_CMD); //关闭显示
    OLED_WR_Byte(0xD5,OLED_CMD); //设置时钟分频因子,震荡频率
    OLED_WR_Byte(80,OLED_CMD);   //[3:0],分频因子;[7:4],震荡频率
    OLED_WR_Byte(0xA8,OLED_CMD); //设置驱动路数
    OLED_WR_Byte(0X3F,OLED_CMD); //默认0X3F(1/64) 
    OLED_WR_Byte(0xD3,OLED_CMD); //设置显示偏移
    OLED_WR_Byte(0X00,OLED_CMD); //默认为0

    OLED_WR_Byte(0x40,OLED_CMD); //设置显示开始行 [5:0],行数.
                                                        
    OLED_WR_Byte(0x8D,OLED_CMD); //电荷泵设置
    OLED_WR_Byte(0x14,OLED_CMD); //bit2，开启/关闭
    OLED_WR_Byte(0x20,OLED_CMD); //设置内存地址模式
    OLED_WR_Byte(0x02,OLED_CMD); //[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
    OLED_WR_Byte(0xA1,OLED_CMD); //段重定义设置,bit0:0,0->0;1,0->127;
    OLED_WR_Byte(0xC0,OLED_CMD); //设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
    OLED_WR_Byte(0xDA,OLED_CMD); //设置COM硬件引脚配置
    OLED_WR_Byte(0x12,OLED_CMD); //[5:4]配置
         
    OLED_WR_Byte(0x81,OLED_CMD); //对比度设置
    OLED_WR_Byte(0xEF,OLED_CMD); //1~255;默认0X7F (亮度设置,越大越亮)
    OLED_WR_Byte(0xD9,OLED_CMD); //设置预充电周期
    OLED_WR_Byte(0xf1,OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
    OLED_WR_Byte(0xDB,OLED_CMD); //设置VCOMH 电压倍率
    OLED_WR_Byte(0x30,OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

    OLED_WR_Byte(0xA4,OLED_CMD); //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
    OLED_WR_Byte(0xA6,OLED_CMD); //设置显示方式;bit0:1,反相显示;0,正常显示	    						   
    OLED_WR_Byte(0xAF,OLED_CMD); //开启显示	 
    OLED_Clear();

    printf("OLED ssd1306 init\r\n");
    return;
}

#endif


