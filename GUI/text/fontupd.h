#ifndef __FONTUPD_H__
#define __FONTUPD_H__	 
#include <stm32f10x.h>
#include "util.h"

//前面4.8M被fatfs占用了.
//4.8M以后紧跟的100K字节,用户可以随便用.
//4.8M+100K字节以后的字节,被字库占用了,不能动!

#define FONT_UNICODE2GBK         0
#define FONT_GBK12               1
#define FONT_GBK16               2
#define FONT_GBK24               3

#define FONT_GBK12_SIZE         12
#define FONT_GBK14_SIZE         14
#define FONT_GBK16_SIZE         16


//字体信息保存地址,占33个字节,第1个字节用于标记字库是否存在.后续每8个字节一组,分别保存起始地址和文件大小														   
extern u32 FONTINFOADDR;	
//字库信息结构体定义
//用来保存字库基本信息，地址，大小等
__packed typedef struct 
{
    u8 fontok;				//字库存在标志，0XAA，字库正常；其他，字库不存在
    u32 ugbkaddr; 			//unigbk的地址
    u32 ugbksize;			//unigbk的大小	 
    u32 f12addr;			//gbk12地址	
    u32 gbk12size;			//gbk12的大小	 
    u32 f16addr;			//gbk16地址
    u32 gbk16size;			//gbk16的大小		 
    u32 f24addr;			//gbk24地址
    u32 gkb24size;			//gbk24的大小 
}_font_info; 


extern _font_info ftinfo;	//字库信息结构体


u32 fupd_prog(u32 fsize, u32 pos); //显示更新进度
u8 updata_fontx(u8 *fxpath, u8 fx); //更新指定字库
u8 update_font(void); //更新全部字库
u8 font_init(void); //初始化字库

#endif





















