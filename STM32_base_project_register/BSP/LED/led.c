#include "sys.h"   
#include "led.h"
#include "delay.h"

//
//
/**
 * @brief 初始化PA8和PD2为输出口.并使能这两个口的时钟		    
 * 
 * @desc: LED IO初始化
 * 		  配置 GPIO
 */
void LED_Init(void)
{
	//使能时钟
	RCC->APB2ENR |= 1 << 2;    //使能PORTA时钟	   	 
	RCC->APB2ENR |= 1 << 5;    //使能PORTD时钟	   	 
	GPIOA->CRH &= 0XFFFFFFF0;  //清零, 0xF0 : 1111 0000
	GPIOA->CRH |= 0X00000003;  //PA8 推挽输出   	 
    GPIOA->ODR |= 1 << 8;      //PA8 输出高
											  
	GPIOD->CRL &= 0XFFFFF0FF;
	GPIOD->CRL |= 0X00000300;	//PD.2推挽输出
	GPIOD->ODR |= 1 << 2;      //PD.2输出高 

	return;
}

/**
 * @brief LED set
 * 
 */
void LED_Set()
{
	GPIO_Set(GPIOA, 8, 1);
	GPIO_Set(GPIOD, 2, 1);
	
	return;
}

/**
 * @brief LED reset
 * 
 */
void LED_Reset()
{
	GPIO_Reset(GPIOA, 8, 0);
	GPIO_Reset(GPIOD, 2, 0);
	
	return;
}


/**
 * @brief 将 第 ix 位置位 
 * 
 * @param gpio 
 * @param pin_num 
 * @param val 
 */
void GPIO_Set(GPIO_TypeDef *gpio, uint8_t pin_num, uint8_t val)
{
	u32 temp = gpio->ODR;		//暂存原来的值
	temp |= val << pin_num;
	gpio->ODR |= temp;

	return;
}

/**
 * @brief reset GPIO pin
 * 
 * @param gpio 
 * @param pin_num 
 * @param val 
 */
void GPIO_Reset(GPIO_TypeDef *gpio, uint8_t pin_num, uint8_t val)
{
	u32 temp = gpio->ODR;			//暂存原来的值
	temp &= ~(1 << pin_num);	//~ 按位取反
	gpio->ODR |= temp;

	return;
}





