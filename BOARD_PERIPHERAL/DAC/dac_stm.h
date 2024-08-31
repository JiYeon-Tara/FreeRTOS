#ifndef __DAC_H
#define __DAC_H	 
#include "sys.h"	    


/*****************************************************************************
 * MACRO
 *****************************************************************************/
#define DAC_OUTPUT_MAX_VOL      3300    // mv, DAC 最大输出 3.3V

/**
 * @brief DAC通道1输出初始化
 * 
 */
void Dac1_Init(void);

/**
 * @brief 设置通道1输出电压
 * 	      DAC 的输出范围:0-3.3
 * 
 * @param vol vol:0~3300,代表0~3.3V
 */
void Dac1_Set_Vol(u16 vol);


#endif

















