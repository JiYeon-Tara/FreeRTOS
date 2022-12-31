#ifndef __SSD_13306_H__
#define __SSD_13306_H__	




#if OLED_SCREEN_ENABLE

#include "sys.h"
#include "stdlib.h"
#include "bsp_config.h"
#include "oled.h"   
//////////////////////////////////////////////////////////////////////////////////
//1,修改OLED_ShowChar函数,支持12/16/24字体
//2,修改OLED_ShowString函数,支持字体设置
////////////////////////////////////////////////////////////////////////////////// 	  


//OLED控制用函数
// void OLED_WR_Byte(u8 dat,u8 cmd);	    
// void OLED_Display_On(void);
// void OLED_Display_Off(void);
// void OLED_Refresh_Gram(void);		   
                                           
// void OLED_Init(void);
// void OLED_Clear(void);
// void OLED_DrawPoint(u8 x,u8 y,u8 t);
// void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);
// void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode);
// void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);
// void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size);	 
#endif // OLED_SCREEN_ENABLE


#endif // __SSD_13306_H__
     



