#include "timer.h"
#include "led.h"

/**
 * @brief 通用定时器中断初始化
 *        这里时钟选择为APB1的2倍，而APB1为36M
 * @param[in]  arr 自动重装值
 * @param[in]  psc 时钟预分频数
 */
void TIM3_Int_Init(u16 arr, u16 psc)
{
	RCC->APB1ENR |= 1<<1;	//TIM3时钟使能    
 	TIM3->ARR = arr;  	//设定计数器自动重装值 
	TIM3->PSC = psc;  	//预分频器设置
	TIM3->DIER |= 1<<0;   //允许更新中断				
	TIM3->CR1 |= 1<<0;    //使能定时器3
  	MY_NVIC_Init(TIM3_PRIEMPTION_PRIORITY, TIM3_SUB_PRIORITY, TIM3_IRQn, TIM3_NVIC_GROUP); //抢占1，子优先级3，组2									 
}

/**
 * @brief 定时器3中断服务程序
 * 
 */
void TIM3_IRQHandler(void)
{ 		    		  			    
	if(TIM3->SR & 0X0001)//溢出中断
	{
		LED1 =! LED1;			    				   				     	    	
	}				   
	TIM3->SR &= ~(1<<0);//清除中断标志位 	    
}

