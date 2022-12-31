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

    printf("led red and yellow init\r\n");
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

/**
 * @brief change led status, service layer
 * 
 * @param state 
 */
bool led_state_change(LED_PIN_E led, LED_STATUS_E status)
{
    GPIO_TypeDef *gpio;
    uint8_t pinNum;

    switch(led){
        case LED_0:
            gpio = GPIOA;
            pinNum = LED0_PORT_NUM;
            break;
        case LED_1:
            gpio = GPIOD;
            pinNum = LED1_PORT_NUM;
            break;
        default:
            printf("unknown led:%d\r\n", led);
            // break;
            return false;
    }

    if(status == LED_EXTINCT){
        GPIO_Set(gpio, pinNum);
    }
    else if(status == LED_LIGHT){
        GPIO_Reset(gpio, pinNum);
    }
    else{
        printf("unknown status:%d\r\n", status);
        return false;
    }

    return true;
}





