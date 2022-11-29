#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
#include "board_config.h"  
#include <stdbool.h>


// macro
// 方法1: 位带操作, #include "sys.h"
#define LED0 PAout(8)	// PA8
#define LED1 PDout(2)	// PD2	

// Power on/off GPIO level, 共阴极/共阳极
#define LED_PWR_ON		0   	// low level pwron led
#define LED_PWR_OFF	    1

// typedef
typedef enum{
    LED_0,
    LED_RED = LED_0,
    LED_1,
    LED_YELLOW = LED_1,
    LED_NUM
} LED_PIN_E;

typedef enum{
    // The idea of cloning extinct life forms still belongs to science fiction.
    LED_EXTINCT,
    LED_LIGHT,
} LED_STATUS_E;





// 方法2
#define LED0_SET(x)     GPIOA->ODR=(GPIOA->ODR&~LED0) | (x ? LED0 : 0)
#define LED1_SET(x)     GPIOD->ODR=(GPIOD->ODR&~LED0) | (x ? LED1 : 0)

void LED_Init(void);	//初始化	
void LED_Set(void);
void LED_Reset(void);
void LED_Toggle(void);

bool led_state_change(LED_PIN_E led, LED_STATUS_E status);

#endif

















