#ifndef __ADC_H__
#define __ADC_H__

#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"

/************
 * MACRO
 ************/
#define ADC_VAL_CH0                 0 // 通道0	
#define ADC_VAL_CH1                 1 // 通道1		
#define ADC_GET_TEST_COUNT          1 // 100
#define ADC_MCU_TEMPER_ENABLE       1 // 使用 ADC 检测 MCU 自带的温度传感器测量温度(内部温度传感器)
#define ADC_MCU_TEMP_CHANNEL        16 // 内部温度传感器和 ADC1 channel 16 相连

	   									   
void Adc_Init(void); 				//ADC通道初始化
u16  Get_Adc(u8 ch); 				//获得某个通道值 
u16 Get_Adc_Average(u8 ch,u8 times);//得到某个通道10次采样的平均值

/**
 * @brief 测量到的 ADC 的值与电压值之前的量程换算
 * 	      线性
 * 	      如果得到的是其他非线性的数据, 还需要复杂的换算
 * @param adcVal:0-4096
 * @ret   0 - 3.3
 */
float adcVal2Voltage(uint16_t adcVal);

/**
 * @brief 通过 ADC的值计算温度传感器温度
 * 	      一次函数关系: 温度与电压, 先计算电压
 * 		  典型值: 25度时， 电压值:1.43
 * @param adcVal
 * @ret	  temperature
 */
float adcVal2Temper(uint16_t adcVal);

#endif 









#endif

