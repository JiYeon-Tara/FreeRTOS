#include "gpio_stm.h"

#define GPIO_SET		1
#define GPIO_RESET		0

// 4种输入模式：
//        输入浮空
//        输入上拉
//        输入下拉
//        模拟输入
// 4种输出模式：
//       开漏输出
//       开漏复用功能
//       推挽式输出
//       推挽式复用功能

/**
 * @brief GPIO set val
 * 
 * @param gpio 
 * @param pin_num port num
 * @param val 
 */
void GPIO_Set(GPIO_TypeDef *gpio, uint8_t pin_num)
{
	u32 temp = gpio->ODR; // 取出原来的这个 GPIO 各个端口的输出情况
	temp |= (GPIO_SET << pin_num); // 仅仅改变需要修改的 port
	gpio->ODR |= temp; // 修改 GPIO->ODR
	return;
}

/**
 * @brief reset GPIO pin
 * 
 * @param gpio 
 * @param pin_num 
 * @param val 
 */
void GPIO_Reset(GPIO_TypeDef *gpio, uint8_t pin_num)
{
	u32 temp = gpio->ODR; // 取出原来的这个 GPIO 各个端口的输出情况
	temp &= ~(GPIO_SET << pin_num); // 将这一位之外的其他值都不变, 仅仅将这一位清 0
	gpio->ODR &= temp; // 修改 GPIO->ODR
	return;
}

/**
 * @brief toggle GPIO pin
 * 
 * @param gpio 
 * @param pin_num 
 * @param val 
 */
void GPIO_Toggle(GPIO_TypeDef *gpio, uint8_t num)
{
	u32 temp = gpio->ODR;
	
	if ((temp >> num) & 0x01) { // 1
		temp &= ~(GPIO_SET << num); // reset
	} else { // 0
		temp |= (GPIO_SET << num);
	}

	gpio->ODR = temp;
	// printf("bit[%d]:", num);
	// printf("temp:%#08X\n", temp);

	return;
}
