/**
 * @file ssh_1106.c(中景园电子 ssd_1106.c)
 * @author your name (you@domain.com)
 * @brief 代码框架整理, 与芯片相关的分别放到不同的文件夹下, OLED 都放在 oled.c/oled.h 中
 *        供应商提供
 *
 * @version 0.1
 * @date 2022-12-18
 *
 * @copyright Copyright (c) 2022
 * 
 */


// VCC_IN:
// GND
// D0:SPI 接口是为 SPI 时钟线, IIC 时钟线;
// D1:SPI 接口时为 SPI 数据线, IIC 数据线;
// RES:OLED 复位, OLED 上电后需要做一个复位
// DC:SPI 数据/命令选择引脚, IIC 时用来设置 IIC 地址

#include "stdlib.h"
#include "oledfont.h"  	 
#include "delay.h"
#include "oled.h"
#include "ssh_1106.h"
#include "usart.h"
#include "ulog.h"


// OLED screen hardware parameter
#define SIZE        16 // font_size, 一个字符的大小:32/16/12, font size 与字库对应关系
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	    		

// command or data
#define OLED_CMD    0	// 写命令
#define OLED_DATA   1	// 写数据


// SSH1106 显存:
//OLED的显存 大小:128 * 64 像素点: 128*8 bytes
// uint8_t OLED_GRAM[8][128]; // row(y):0-7, col:0-127(x)
// 列地址(x):0-127, 行地址(y):0-7

//存放格式如下.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127
// OLED 字体大小与显存的关系: 
//对于 16*16 的字符, 一个字符占用两行 page, 由上半部和下半部组成
//对于 32*32 的中文, 一个字符占用 4 行 page, 从字库也可以看出


//ssh_1106 支持的命令:
// 其他命令，我们就不在这里一一介绍了，大家可以参考 SSD1306 datasheet 的第 28 页。从
// 这页开始，对 SSD1306 的指令有详细的介绍。

#if OLED_MODE == OLED_8080
/**
 * @brief 向SSD1106写入一个字节。
 * 
 * @param dat 要写入的数据/命令
 * @param cmd 数据/命令标志 0,表示命令;1,表示数据;
 */
void OLED_WR_Byte(u8 dat,u8 cmd)
{
    DATAOUT(dat);	    
    if(cmd)
      OLED_DC_Set();
    else 
      OLED_DC_Clr();		   
    OLED_CS_Clr();
    OLED_WR_Clr();	 
    OLED_WR_Set();
    OLED_CS_Set();
    OLED_DC_Set();	 
}
#else
/**
 * @brief 通过 SPI 向 SSD1106 写入一个字节
 * 
 * @param dat 要写入的数据/命令
 * @param cmd 数据/命令标志, 0:表示命令; 1:表示数据;
 */
void OLED_WR_Byte(u8 data, u8 cmd)
{
    if(cmd == OLED_DATA) // command/data
        OLED_DC_Set();
    else 
        OLED_DC_Clr();

    SPI1_Soft_Write(data);
    OLED_DC_Set(); // 恢复 DC

    return;
}
#endif

/**
 * @brief 设置要显示的位置
 *
 * @param x 0~127
 * @param y 0~7 设置页地址, 其低三位的值对应着 GRAM 的页地址
 */
void OLED_Set_Pos(unsigned char x, unsigned char y) 
{
    // OLED 交互命令:0xB0 - 0xB7 为要设置的页地址, 低三位:0x00-0x07
    OLED_WR_Byte(0xb0 + y, OLED_CMD);

    // 命令参数:
    // (x + 2) & (1111 0000)b >> 4 | 0x10
    // x 轴偏移 2 ——开头的两个像素点空开, 不显示字符
    OLED_WR_Byte((((x + 2) & 0xf0) >> 4) | 0x10, OLED_CMD); // 设置列地址高四位, 该指令用于设置显示时的起始列地址高四位:0x10-0x1F
    OLED_WR_Byte(((x + 2) & 0x0f), OLED_CMD); // 0x00-0x0F, 列地址的低四位, 该指令用于设置显示时的起始列地址低四位:0x00-0x0F
}

//开启OLED显示    
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC命令, 打开电荷泵
    OLED_WR_Byte(0X14, OLED_CMD);  //DCDC ON, 0x14:00010100
    OLED_WR_Byte(0XAF, OLED_CMD);  //DISPLAY ON, 0xAF:10101111
}

//关闭OLED显示     
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC命令
    OLED_WR_Byte(0X10, OLED_CMD);  //DCDC OFF, 0x10:00010000
    OLED_WR_Byte(0XAE, OLED_CMD);  //DISPLAY OFF, 0xAE:10101110
}		

/**
 * @brief 清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
 * 
 */
void OLED_Clear(void)  
{
    u8 i,n;		

    // 0 - 7 页依次清除
    // 分辨率:128 * 64 -> GRAM:128 * 8 bytes
    for (i = 0; i < 8; i++) {
        OLED_WR_Byte (0xb0 + i, OLED_CMD); //设置页地址（0~7）-> 0xB0-0xB7
        // 设置起始地址
        OLED_WR_Byte (0x02, OLED_CMD); //低四位:0010b:设置显示位置-列低地址, 显示时, 起始列地址的低四位:0x00-0x0F
        OLED_WR_Byte (0x10, OLED_CMD); //高四位:0000b:设置显示位置—列地址高四位, 显示时, 起始列地址的低高位:0x10-0x1F
        // 显示时, 起始位置列地址:00000010
        for (n = 0; n < 128; n++) // 一页 128 bytes
            OLED_WR_Byte(0, OLED_DATA); 
    } //更新显示

    return;
}

/**
 * @brief 在指定位置显示字符
 * 
 * @param x 0~127 在指定位置显示一个字符,包括部分字符
 * @param y 0~7 行数, 显存的页数, 每一页的宽度是 8 个像素点
 * @param chr 
 * @param mode 0,反白显示;1,正常显示	
 * @param SIZE 选择字体 16/12
 */
void OLED_ShowChar(u8 x, u8 y, u8 chr)
{
    unsigned char char_idx = 0, i = 0;	
    char_idx = chr - ' ';//得到字符相对于 ' ' 偏移后的值

    if (x > Max_Column - 1) { // 自动换行
        x = 0; 
        y = y + 1; // 换行, y + 1
        // y = h + 2; // 每行之间空开一页, 总共有 8 页:page[0] - page[7]
    }

    if (SIZE == 16) { // font size = 16, 每一个字符都占了 OLED_GRAM 的两页, 上半部和下半部合并组成一个字符
        OLED_Set_Pos(x, y); // 上半部 在 page[y]
        for(i = 0; i < 8; i++)
            OLED_WR_Byte(F8X16[char_idx * 16 + i], OLED_DATA);

        OLED_Set_Pos(x, y + 1); // 下半部在 page[y+1]
        for(i = 0; i < 8; i++)
            OLED_WR_Byte(F8X16[char_idx * 16 + i + 8], OLED_DATA);
    } else { // font size = 12;
        OLED_Set_Pos(x, y + 1);
        for(i = 0; i < 6; i++)
            OLED_WR_Byte(F6x8[char_idx][i], OLED_DATA);
    }

    return;
}

/**
 * @brief m^n函数
 * 
 * @param m 
 * @param n 
 * @return u32 
 */
u32 oled_pow(u8 m,u8 n)
{
    u32 result=1;	 
    while(n--)result*=m;    
    return result;
}

/**
 * @brief 显示2个数字
 * 
 * @param x 起点坐标	 
 * @param y 
 * @param num 数值(0~4294967295)
 * @param len 数字的位数
 * @param size 字体大小; 
 *      模式 0,填充模式;1,叠加模式
 * @return * void 
 */
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size)
{
    u8 t, temp;
    u8 enshow = 0;
					   
    for (t = 0; t < len; t++) {
        temp = (num / oled_pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                OLED_ShowChar(x+(size/2)*t,y,' ');
                continue;
            } else
                enshow=1; 
        }
        OLED_ShowChar(x + (size / 2) * t, y, (temp + '0')); 
    }
}

/**
 * @brief 显示一个字符号串
 * 
 * @param x 0~127
 * @param y 0-7, y 表示页??
 * @param str must end with '\0'
 */
void OLED_ShowString(u8 x, u8 y, u8 *str)
{
    unsigned char j=0;

    while (str[j] != '\0') {
        OLED_ShowChar(x, y, str[j]);
        x += 8;
        if (x > 120) { // 换行???, 
            x = 0;
            y += 2; // 行之间, 空开两个像素(bit)的宽度
        }
        j++;
        LOG_D("x:%d, y:%d", x, y);
    }
}

/**
 * @brief 中文显示 16*16 汉字(占用 2 pages)
 *        显示汉字
 *        这种方式只能显示指定的汉字
 * 
 * @param x 0-127, OLED_GRAM column index
 * @param y:0-7 OLED_GRAM page index(row)
 * @param charIdx 汉字在字库 Hzk 中的位置索引 index
 */
void OLED_ShowCHinese(u8 x, u8 y, u8 charIdx)
{
    u8 t, adder=0;

    OLED_Set_Pos(x, y);	// page[y]
    for(t = 0; t < 16; t++){
        OLED_WR_Byte(chuini[2 * charIdx][t], OLED_DATA);
        adder += 1;
    }
    OLED_Set_Pos(x, y+1); // page[y+1]
    for(t = 0; t < 16; t++){	
        OLED_WR_Byte(chuini[2 * charIdx + 1][t], OLED_DATA);
        adder += 1;
    }
    return;		
}

/**
 * @brief 中文显示 32*32 的汉字
 * 
 * @param x 
 * @param y 
 * @param no 
 */
void OLED_ShowCHinese32x32(u8 x, u8 y, u8 no)
{ 
    u8 t;

    OLED_Set_Pos(x, y);
    for (t = 0; t < 32; t++) {
        OLED_WR_Byte(Hzk32x32[4*no][t],OLED_DATA);
    }

    OLED_Set_Pos(x, y+1);
    for (t = 0; t < 32; t++) {
        OLED_WR_Byte(Hzk32x32[4*no+1][t], OLED_DATA);
    }

    OLED_Set_Pos(x, y+2);
    for (t = 0; t < 32; t++) {
        OLED_WR_Byte(Hzk32x32[4*no+2][t],OLED_DATA);
    }

    OLED_Set_Pos(x, y+3);
    for (t = 0; t < 32; t++) {
        OLED_WR_Byte(Hzk32x32[4*no+3][t],OLED_DATA);
    }
}


/**
 * @brief 显示显示BMP图片 128 × 64
 * 
 * @param x0 起始点坐标(x,y), x的范围0～127
 * @param y0 
 * @param x1 ，y为页的范围0～7*
 * @param y1 
 * @param BMP 
 */
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{
    unsigned int j=0;
    unsigned char x,y;

    if(y1 % 8 == 0)
        y = y1 / 8;
    else
        y = y1 / 8 + 1;
    for (y = y0; y < y1; y++) {
        OLED_Set_Pos(x0, y);
        for (x = x0; x < x1; x++) {      
            OLED_WR_Byte(BMP[j++], OLED_DATA);	    	
        }
    }

    return;
}


/**
 * @brief 初始化 SSD1306
 * 
 * @return * void 
 */
void OLED_Init(void)
{
    LOG_I("OLED ssh1106 init");
#if OLED_BSP_HAL /* HAL 库版本 */
    GPIO initialization with HAL lib
 	GPIO_InitTypeDef  GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOG, ENABLE);	 //使能PC,D,G端口时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_3|GPIO_Pin_8;	 //PD3,PD6推挽输出  
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOD, &GPIO_InitStructure);	  //初始化GPIOD3,6
 	GPIO_SetBits(GPIOD,GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_3|GPIO_Pin_8);	//PD3,PD6 输出高

#if OLED_MODE == OLED_8080
 	GPIO_InitStructure.GPIO_Pin =0xFF; //PC0~7 OUT推挽输出
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOC,0xFF); //PC0~7输出高
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15; //PG13,14,15 OUT推挽输出
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOG,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15); //PG13,14,15 OUT  输出高
#else
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1; //PC0,1 OUT推挽输出
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOC,GPIO_Pin_0|GPIO_Pin_1); //PC0,1 OUT  输出高
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15; //PG15 OUT推挽输出	  RST
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOG,GPIO_Pin_15); //PG15 OUT  输出高
#endif // OLED_MODE
#else /* 寄存器版本 */
    RCC->APB2ENR |= 1 << 3; // 使能PORTB时钟 
    RCC->APB2ENR |= 1 << 4; // 使能PORTC时钟 	  
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
#if OLED_RESET_GPIO_CTR
    GPIOB->CRH &= 0xF0FFFFFF; // PB14 clear
    GPIOB->CRH |= 0x03000000; // output push pull
    GPIOB->ODR |= (1 << 14);  // PB[14] output 1
#endif /*OLED_RESET_GPIO_CTR*/
    SPI1_Soft_Init();
#endif // OLED_MODE
#endif // OLED_BSP_HAL

    OLED_RST_Set();
    delay_ms(100);
    OLED_RST_Clr();
    delay_ms(100);
    OLED_RST_Set(); 

    OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
    OLED_WR_Byte(0x02,OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register, 设置对比度
    OLED_WR_Byte(0xCF,OLED_CMD); // Set SEG Output Current Brightness, 设置的对比度的值, 值越大, 屏幕月亮
    OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    OLED_WR_Byte(0x00,OLED_CMD);//-not offset
    OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
    OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
    OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
    OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
    OLED_WR_Byte(0x12,OLED_CMD);
    OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
    OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
    OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
    OLED_WR_Byte(0x02,OLED_CMD);//
    OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
    OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
    OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
    OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7) 
    OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel, 设置显示开关
    
    OLED_WR_Byte(0xAF,OLED_CMD); /*display ON*/ 
    OLED_Clear();
    OLED_Set_Pos(0, 0); 	

    return;
}  

