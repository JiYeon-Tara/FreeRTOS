#include "dac_stm.h"
#include "ulog.h"
/*****************************************************************************
 * PUBLIC FUNCTIONS
 *****************************************************************************/
/**
 * @brief DAC通道1输出初始化
 * 两个 DAC 对应两个输出通道, PA4, PA5
 * 
 */
void Dac1_Init(void)
{
    LOG_I("%s enter", __func__);
    RCC->APB2ENR |= 1 << 2; //使能PORTA时钟	  	
    RCC->APB1ENR |= 1 << 29; //使能DAC时钟	  	

    GPIOA->CRL &= 0XFFF0FFFF; // clear
    GPIOA->CRL |= 0X00000000; // PA4 模拟输入,

    DAC->CR |= 1 << 0; //使能DAC1
    DAC->CR |= 1 << 1; //DAC1输出缓存不使能 BOFF1=1
    DAC->CR |= 0 << 2; //不使用触发功能 TEN1=0
    DAC->CR |= 0 << 3; //DAC TIM6 TRGO,不过要TEN1=1才行
    DAC->CR |= 0 << 6; //不使用波形发生
    DAC->CR |= 0 << 8; //屏蔽、幅值设置
    DAC->CR |= 0 << 12;	//DAC1 DMA不使能    
    DAC->DHR12R1 = 0; // 数据清零,12 位右对齐数据
    LOG_I("%s exit", __func__);
}

/**
 * @brief 设置通道1输出电压
 * 	      DAC 的输出范围:0-3.3
 * 
 * @param vol vol:0~3300, 代表0~3.3V
 */
void Dac1_Set_Vol(u16 vol)
{
    float temp = vol;
    temp /= 1000;
    temp = temp * 4096 / 3.3;
    DAC->DHR12R1 = (uint32_t)temp;
}

/**
 * @brief DAC 输出三角波
 * 		  阻塞, 不可以进行其他操作
 * 
 */
void dac_output_triangular()
{
    int ix;
    for(ix = 0; ix < DAC_OUTPUT_MAX_VOL; ++ix){
        Dac1_Set_Vol(ix);
        delay_ms(1);
    }
    for(; ix > 0; --ix){
        Dac1_Set_Vol(ix);
        delay_ms(1);
    }
}





















































