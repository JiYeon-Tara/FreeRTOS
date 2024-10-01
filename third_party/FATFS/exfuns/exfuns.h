#ifndef __EXFUNS_H
#define __EXFUNS_H 			   
#include <stm32f10x.h>
#include "ff.h"
#include "util.h"
/*********************************************************************************
 * MACRO
 *********************************************************************************/
//f_typetell返回的类型定义
//根据表FILE_TYPE_TBL获得.在exfuns.c里面定义
#define T_BIN		0X00	//bin文件
#define T_LRC		0X10	//lrc文件
#define T_NES		0X20	//nes文件
#define T_TEXT		0X30	//.txt文件
#define T_C			0X31	//.c文件
#define T_H			0X32    //.h文件
#define T_FLAC		0X4C	//flac文件
#define T_BMP		0X50	//bmp文件
#define T_JPG		0X51	//jpg文件
#define T_JPEG		0X52	//jpeg文件		 
#define T_GIF		0X53	//gif文件

// 通过 f_typetell() 返回值判断文件类型
// 0XFF,表示无法识别的文件类型索引.
// 其他,高四位表示所属大类,低四位表示所属小类.
#define FTYPE_IS_BIN(type) ((type >> 4) == 0x00 ? true : false)
#define FTYPE_IS_LRC(type) ((type >> 4) == 0x01 ? true : false)
#define FTYPE_IS_NES(type) ((type >> 4) == 0x02 ? true : false)
#define FTYPE_IS_TEXT(type) ((type >> 4) == 0x03 ? true : false)
#define FTYPE_IS_AUDIO(type) ((type >> 4) == 0x04 ? true : false)
#define FTYPE_IS_IMAGE(type) ((type >> 4) == 0x05 ? true : false)
/*********************************************************************************
 * PUBLIC VARIABLES
 *********************************************************************************/
extern FATFS *fs[2];  
extern FIL *file;	 
extern FIL *ftemp;	 
extern UINT br,bw;
extern FILINFO fileinfo;
extern DIR dir;
extern u8 *fatbuf;//SD卡数据缓存区
/*********************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************/
u8 exfuns_init(void); //申请内存
u8 f_typetell(u8 *fname); //识别文件类型
u8 exf_getfree(u8 *drv,u32 *total,u32 *free); //得到磁盘总容量和剩余容量
u32 exf_fdsize(u8 *fdname); //得到文件夹大小			 																		   
#endif


