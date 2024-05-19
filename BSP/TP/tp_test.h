#ifndef __TP_TEST_H__
#define __TP_TEST_H__

#include "bsp_config.h"

#if TP_ENABLE

void Load_Drow_Dialog(void);
void gui_draw_hline(u16 x0, u16 y0, u16 len, u16 color);
void gui_fill_circle(u16 x0, u16 y0, u16 r, u16 color);
u16 my_abs(u16 x1,u16 x2);
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color);
void rtp_test(void);
void enter_tp_adjust();
void ctp_test(void);

#endif // TP_ENABLE

#endif

