#ifndef __GPIO_H__
#define __GPIO_H__

#include "stm32f10x.h"
#include "board_config.h"


/**
 * @brief GPIO set val
 * 
 * @param gpio 
 * @param pin_num port num
 * @param val 
 */
void GPIO_Set(GPIO_TypeDef *gpio, uint8_t pin_num);

/**
 * @brief reset GPIO pin
 * 
 * @param gpio 
 * @param pin_num 
 * @param val 
 */
void GPIO_Reset(GPIO_TypeDef *gpio, uint8_t pin_num);

/**
 * @brief toggle GPIO pin
 * 
 * @param gpio 
 * @param pin_num 
 * @param val 
 */
void GPIO_Toggle(GPIO_TypeDef *gpio, uint8_t num);

#endif

