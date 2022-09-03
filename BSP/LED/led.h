#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#include "board_config.h"  

// 方法1: 位带操作, #include "sys.h"
#define LED0 PAout(8)	// PA8
#define LED1 PDout(2)	// PD2	


// 方法2
#define LED0_SET(x)     GPIOA->ODR=(GPIOA->ODR&~LED0) | (x ? LED0 : 0)
#define LED1_SET(x)     GPIOD->ODR=(GPIOD->ODR&~LED0) | (x ? LED1 : 0)

void LED_Init(void);	//初始化	
void LED_Set(void);
void LED_Reset(void);
void LED_Toggle(void);


#endif

















