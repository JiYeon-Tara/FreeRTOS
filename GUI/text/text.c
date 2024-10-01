#include "text.h"	

#include "sys.h" 
#include "fontupd.h"
#include "w25q64.h"
#include "ILI93xx.h"
#include "string.h"
#if FATFS_ENABLE
#include "ff.h"
#endif
#if MEMORY_MANAGE_ENABLE
#include "malloc.h"
#endif

#include <wchar.h> // 链接选项中需要增加 -lwchar
/*********************************************************************************
 * MACRO
 *********************************************************************************/
enum FONT_TYPE_E {
    FONT_TYPE_UTF8 = 0,
    FONT_TYPE_GB2312 = 1,
    FONT_TYPE_GBK = 2,
    FONT_TYPE_MAX
} ;
/*********************************************************************************
 * PROTOTYPE
 *********************************************************************************/
#if 0
WCHAR ff_convert_ext_falsh (WCHAR	chr, UINT	dir);
#else
extern WCHAR ff_convert (WCHAR	chr, UINT	dir);
#endif
static void utf82gbk(WCHAR *str);
/*********************************************************************************
 * PRIVATE VARIABLES
 *********************************************************************************/
static enum FONT_TYPE_E source_font_type = FONT_TYPE_UTF8;
/*********************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************/
/**
 * @brief 从字库中查找出字模(汉字的点阵数据)
 * 
 * @param[in] code GBK 编码字符串的开始地址(字符指针开始),(2bytes, 分高低字节, 大端)
 * @param[out] mat 点阵数据
 * @param[in] size 字体大小, 还需要通过字体大小判断在 flash 中的偏移地址
 */
void Get_HzMat(unsigned char *code, unsigned char *mat, u8 size)
{
    unsigned char qh,ql;
    unsigned char i;					  
    unsigned long foffset; 
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size); //得到字体一个字符对应点阵集所占的字节数

    qh = *code; // MSB First?
    ql = *(++code);
    if (qh < 0x81 || ql < 0x40 || ql == 0xff || qh==0xff) { //非常用汉字, 不在 GBK 编码范围内
        for (i = 0; i < csize; i++) //填充满格,类似于马赛克效果, 也可以显示方框
            *mat++ = 0x00;
        return; //结束访问
    }
    if(ql < 0x7f)
        ql -= 0x40;//注意!
    else
        ql -= 0x41;
    qh -= 0x81;
    foffset = ((unsigned long)190 * qh + ql) * csize;	//得到字库中的字节偏移量  		  
    switch (size) {
        case 12:
            // Q:读取数据量应该是 csize 吧????
            // A:True
            SPI_Flash_Read(mat, foffset+ftinfo.f12addr, 24);
            break;
        case 16:
            SPI_Flash_Read(mat, foffset+ftinfo.f16addr, 32);
            break;
        case 24:
            SPI_Flash_Read(mat, foffset+ftinfo.f24addr, 72);
            break;
        default:
            LOG_E("unknown font size %d", size);
            break;
    }

    return;
}

/**
 * @brief 显示一个指定大小的汉字
 * 
 * @param x 汉字的坐标(x,y)
 * @param y 
 * @param font GBK 编码, 这里进行转换, 具体大小与编码格式有关
 * @param size 字体大小
 * @param mode 0,正常显示,1,叠加显示
 */
void Show_Font(u16 x, u16 y, u8 *font, u8 size, u8 mode)
{
    u8 temp,t,t1;
    u16 y0=y;
    u8 dzk[72]; // 12/14/16 字符大小, 字库数据都不会大于 72 bytes, 如果要支持其他大小字符, 这里要增大
    u8 csize;

    if (size != 12 && size != 16 && size != 24) { //不支持的size
        LOG_E("unknown font size %d", size);
        return;
    }

    //得到字体一个字符对应点阵集所占的字节数
    // 汉字字体是等宽登高的, ASCII 字符, 宽高比:1:2
    // 一个汉字字符占用的字节数:
    // 12:24; 16:32, 24:72
    csize = (size/8 + ((size%8) ? 1 : 0)) * (size);

    Get_HzMat(font, dzk, size); //得到相应大小的点阵数据
    //TODO:
    // 这里显示的只有 csize 的有效部分
    // 浪费资源多读了数据:
    for (t = 0; t < csize; t++) {
        temp = dzk[t]; //得到点阵数据
        //TODO:
        for (t1 = 0; t1 < 8; t1++) {
            if (temp & 0x80)
                LCD_Fast_DrawPoint(x, y, POINT_COLOR);
            else if(mode==0)
                LCD_Fast_DrawPoint(x, y, BACK_COLOR); 
            temp <<= 1;
            y++;
            if ((y - y0) == size) { // 换下一列
                y=y0;
                x++;
                break;
            }
        }
    }
}

/**
 * @brief 在指定位置开始显示一个字符串, 支持自动换行
 *        主要控制每一个字符要显示的位置
 * 
 * @param x (x,y):起始坐标
 * @param y 
 * @param width 要显示的区域 (width*height 矩形区域, 越界不显示)
 * @param height 
 * @param str 字符串, GBK 编码, end with '\0'
 * @param size 字体大小 12/16/24
 * @param mode 0,非叠加方式;1,叠加方式
 */
void LCD_Show_Str(u16 x,u16 y, u16 width, u16 height, u8 *str,u8 size,u8 mode)
{
    u16 x0 = x;
    u16 y0 = y;							  	  
    u8 bHz = 0; //字符或者中文

    while (*str != '\0') { //数据未结束
        if (!bHz) {
            if (*str > 0x80) //中文 
                bHz=1;
            else { // ASCII 字符:0x00 ~ 0x7F
                if (x > (x0 + width - size / 2)) {//换行
                    y+=size;
                    x=x0;	   
                }
                if (y > (y0 + height - size)) //越界返回
                    break;
                if (*str == '\r' || *str == '\n') { //换行符号
                    y += size;
                    x = x0;
                    str++; 
                } else //有效部分写入
                    LCD_ShowChar(x, y, *str, size, mode);
                str++;
                x += size / 2; // ASCII 字符,为全字的一半 
            }
        } else { //中文 
            bHz = 0;//有汉字库    
            if (x > (x0 + width - size)) {//换行
                y += size;
                x = x0;		  
            }
            if (y > (y0 + height - size))
                break;//越界返回  						     
            Show_Font(x, y, str, size, mode); //显示这个汉字,空心显示 
            str += 2; // GBK 编码一个字符占用 2bytes
            x += size;//下一个汉字偏移,例如:12号字体,汉字宽度:12*12, ASCII:12*6
        }
    }
}

//在指定宽度的中间显示字符串
//如果字符长度超过了len,则用Show_Str显示
//len:指定要显示的宽度			  
void LCD_Show_Str_Mid(u16 x,u16 y,u8 *str,u8 size,u8 len)
{
    u16 strlenth=0;

    strlenth = strlen((const char*)str);
    strlenth *= size / 2;
    if (strlenth > len)
        LCD_Show_Str(x, y, lcddev.width, lcddev.height, str, size, 1);
    else {
        strlenth = (len - strlenth) / 2;
        LCD_Show_Str(strlenth+x, y, lcddev.width, lcddev.height, str, size, 1);
    }
}

// TODO:
// utf-8 字符显示仍然有问题
// https://blog.csdn.net/dolphin98629/article/details/123134081
#if 0
/**
 * @brief 显示 UTF-8 编码的字符串, 转换为 GBK 编码后显示
 *        unicode
 * @param x 
 * @param y 
 * @param width 
 * @param height 
 * @param str WCHAR 类型, end with '\0'
 * @param size 字体大小
 * @param mode 
 */
void lcd_show_unicode_str(u16 x, u16 y, u16 width, u16 height, WCHAR *str, u8 size, u8 mode)
{
    // UTF-8 编码转换为 GBK 编码
    u8 *p_u8_str;
    u8 u8_str_size = 0; // include '\0'
    u8 *p_output;

    utf82gbk(str); // 转换为 unicode 编码

    // TODO: 
    // 编译报错, wcslen 找不到定义
    u8_str_size = sizeof(WCHAR) * wcslen(str) + 1;

    // wchar 类型, 转换为 u8 类型
    p_u8_str = mymalloc(u8_str_size);
    p_output = p_u8_str;

    while (*str != '\0') {
        *p_output = *str & 0xFF; // TODO:低字节在前???
        ++p_output;
        *p_output = (*str >> 8) & 0xFF;
        ++p_output;
        ++str;
    }
    str[u8_str_size] = '\0';

    LCD_Show_Str(x, y, width, height, p_u8_str, size, mode);

    free(str);

    return;
}
#endif

/**
 * @brief 使用 FATFS 中提供的 unicode <-> GBK 编码的方法进行编码转换 
 * 
 * @param[in/out] end with '\0'
 */
static void utf82gbk(WCHAR *str)
{
    while (*str != '\0') {
// #ifdef FLASH_ENABLE
#if 0
        ff_convert_ext_falsh()
#else
        *str = ff_convert(*str, 0);

#endif
        ++str;
    }

    return;
}























          






