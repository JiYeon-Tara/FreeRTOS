#include "sys.h"   
#include "led.h"
#include "delay.h"
#include "stm32f10x.h"
#include "gpio_stm.h"

// NOTE: 目前只是学习 GPIO 的简单使用, 
// 后期可以使用回掉函数的方式， 通过注册驱动接口的方式，实现扩展任意个 LED

// LED
#define LED0_PORT_NUM	8 		// PA8
#define LED1_PORT_NUM	2   	// PD2

// Power on/off GPIO level, 共阴极/共阳极
#define LED0_PWR_ON		0   	// low level pwron led
#define LED0_PWR_OFF	1

// GPIO 的高低电平状态
#define LED_SET			1
#define LED_RESET		0


/**
 * @brief LED init		    
 * 
 * @desc: LED0 - PA8 红; 
 * 		  LED1 - PD2 黄
 * 		  
 */
void LED_Init(void)
{
	// enable clock
	RCC->APB2ENR |= 1 << 2;    // enable GPIOA clock	 
	RCC->APB2ENR |= 1 << 5;    // enable GPIOD clock

	GPIOA->CRH &= 0XFFFFFFF0;  // reset PA8, 0xF0 : 1111 0000
	GPIOA->CRH |= 0X00000003;  // PA8:0011, output mode	 
    GPIOA->ODR |= (1 << 8);    // PA8 output 1
											  
	GPIOD->CRL &= 0XFFFFF0FF;	// reset PD2
	GPIOD->CRL |= 0X00000300;	// output mode
	GPIOD->ODR |= (1 << 2);     // output 1

	return;
}

/**
 * @brief LED set
 * 
 */
void LED_Set()
{
	GPIO_Set(GPIOA, LED0_PORT_NUM);
	GPIO_Set(GPIOD, LED1_PORT_NUM);
	
	return;
}

/**
 * @brief LED reset
 * 
 */
void LED_Reset()
{
	GPIO_Reset(GPIOA, LED0_PORT_NUM);
	GPIO_Reset(GPIOD, LED1_PORT_NUM);
	
	return;
}

/**
 * @brief LED reset
 * 
 */
void LED_Toggle()
{

}






