/**
 * @file adc_stm.c
 * @author your name (you@domain.com)
 * @brief STM32 ADC/DAC/
 * 		  内部 CPU温度传感器可以直接连接到 ADC 的通道上, 可以直接获取到 CPU 温度
 * @version 0.1
 * @date 2022-12-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "adc_stm.h"
#include <stdio.h>

#include "adc_stm.h"
#include "delay.h"
#include "ulog.h"
#include "sys.h"


// ADC 采样可以由软件触发, 也可以由定时器的事件触发
// 可以用于采样 STM32 内部的温度传感器
// STM32F10eRCT6, ADC1 的模拟输入通道16,17分别接在芯片内部的温度传感器和 VreINT
// ADC2 的 16， 17 通道芯片内部接在了 VSS
// ADC3 的 14, 15, 16, 17 与 Vss 相连
// 采样时间应该长一些，以获取较高的准确度，但是会降低转换速率


/**
 * @brief 初始化ADC, 
 *        ADC1, 通道1 - PA1; 通道0 - PA0, 仅使用规则通道组的一个通道，这里我们仅以规则通道为例
 *        管脚复用:PA1/ADC123_IN1/TIM2_CH2/TIM5_CH2, 详细参考:《ADC通道与GPIO对应表》
 * 
 */
void  Adc_Init(void)
{
    LOG_I("%s enter", __func__);
    //先初始化IO口
    RCC->APB2ENR |= 1 << 2; //使能PORTA口时钟 
    GPIOA->CRL &= 0XFFFFFF0F; //PA1 anolog输入
    //通道10/11设置
    RCC->APB2ENR |= 1 << 9; //ADC1时钟使能	  
    RCC->APB2RSTR |= 1 << 9; //ADC1复位
    RCC->APB2RSTR &= ~(1 << 9); //复位结束	    
    RCC->CFGR &= ~(3 << 14); //分频因子清零	
    //SYSCLK/DIV2=12M ADC时钟设置为12M,ADC最大时钟不能超过14M! 否则将导致ADC准确度下降! 
    RCC->CFGR |= 2 << 14; // bit[15:14]:11, PCLK2 6 分频, 36/6=6MHz

    ADC1->CR1 &= ~(0x0F << 16); // bit[19:16] DUALMOD[3:0] 工作模式清零, 0XF0FFFF 
    ADC1->CR1 |= 0 << 16; //独立工作模式
    ADC1->CR1 &= ~(1 << 8); //非扫描模式
    ADC1->CR2 &= ~(1 << 1); //单次转换模式
    ADC1->CR2 &= ~(7 << 17); // clear SWSTART
    ADC1->CR2 |= 7 << 17; //111：SWSTART,软件控制(外部事件)转换
    ADC1->CR2 |= 1 << 20; //使用用外部触发，这里设置为(SWSTART)!!!
    ADC1->CR2 &= ~(1 << 11); //右对齐
#if ADC_MCU_TEMPER_ENABLE
    ADC1->CR2 |= 1 << 23; // 启用温度传感器和VREFINT。使能 MCU 内部的温度传感器
#endif
    ADC1->SQR1 &= ~(0X0F << 20); // clear bit[23:20]
    ADC1->SQR1 |= 0 << 20; //总共1个转换在规则序列中 也就是只转换规则序列1
    //设置通道1的采样时间
    ADC1->SMPR2 &= ~(7 << 3); //通道1采样时间清空
    ADC1->SMPR2 |= 7 << 3; // 采样时间:通道1  239.5周期,提高采样时间可以提高精确度
#if ADC_MCU_TEMPER_ENABLE
    ADC1->SMPR1 &= ~(7 << 18); // 通道 16 采样时间清空
    ADC1->SMPR1 |= (7 << 18);  // 采样时间:通道16, 采样时间239.65 个采样周期
#endif
    ADC1->CR2 |= 1 << 0; //开启 ADC
    ADC1->CR2 |= 1 << 3; //使能复位校准
    while(ADC1->CR2 & (1 << 3)); //等待校准结束, 该位由软件设置并由硬件清除。在校准寄存器被初始化后该位将被清除。
    ADC1->CR2 |= 1 << 2; //开启AD校准
    while(ADC1->CR2&1<<2); //等待校准结束, 该位由软件设置以开始校准，并在校准结束时由硬件清除
    LOG_I("%s exit", __func__);
}


/**
 * @brief 获得ADC值, 因为是独立模式, 所以需要主动获取, 获取完就结束, 
 *        总耗时:采样时间(239.5 *ADC_CLK) + 转换时间(12.5个 ADC_CLK), 
 *        该接口会阻塞
 * 
 * @param ch 通道值 0~16
 * @return u16 转换结果
 */
u16 Get_Adc(u8 ch)   
{
    //设置转换序列	  		 
    ADC1->SQR3 &= 0XFFFFFFE0; // 清零
    ADC1->SQR3 |= (ch << 0); //SQ1[4:0]规则序列中的第一个转换设置为要转换的通道, 0XFFFFFFE0
    ADC1->CR2 |= 1 << 22; //启动规则转换通道 
    while(!(ADC1->SR & 1 << 1)); //等待转换结束

    return (uint16_t)ADC1->DR; //读取 adc 值	
}

//获取通道ch的转换值，取times次,然后平均 
//ch:通道编号
//times:获取次数
//返回值:通道ch的times次转换结果平均值
// 会有阻塞
u16 Get_Adc_Average(u8 ch, u8 times)
{
    u32 temp_val = 0;
    u8 t;
    for(t = 0; t < times; t++)
    {
        temp_val += Get_Adc(ch);
        delay_ms(5);
    }

    return temp_val/times;
}

/**
 * @brief 测量到的 ADC 的值与电压值之前的量程换算
 * 	      线性
 * 	      如果得到的是其他非线性的数据, 还需要复杂的换算
 * @param adcVal:0-4096
 * @ret   0 - 3.3
 */
float adcVal2Voltage(uint16_t adcVal)
{
    return (float)adcVal * 3.3 / 4096;
}

/**
 * @brief 通过 ADC的值计算温度传感器温度
 * 	      一次函数关系: 温度与电压, 先计算电压
 * 		  典型值: 25度时， 电压值:1.43, 参考电气手册
 *        《STM32中文参考手册》 P169, 参考数据手册的电气特性章节中V25 和Avg_Slope的实际值
 * 
 * @param adcVal
 * @ret	  temperature
 */
float adcVal2Temper(uint16_t adcVal)
{
    float vol = adcVal2Voltage(adcVal);
    // LOG_I("adcVal:%d\n", adcVal);
    // LOG_I("vol:%f\n", vol);
    return (float)((1.43 - vol) / 0.0043 + 25);
}







