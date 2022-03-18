#ifndef __STM32F103RCT6_MINI_DEVELOPMENT_BOARD__
#define __STM32F103RCT6_MINI_DEVELOPMENT_BOARD__
#include "sys.h"
/**
 * @brief 使用这个头文件对板上的常用外设进行定义
 *        也可以在 各个模块的驱动文件中分别定义 
 * 
 */
//USART1:组2，最低优先级， 抢占优先级 和 子优先级的值都是2
#define UART1_PRIEMPTION_PRIORITY  3
#define UART1_SUB_PRIORITY         3
#define UART1_NVIC_GROUP           2

//PC5:KEY0
#define KEY0_GPIO                  GPIO_C
#define KEY0_PORT                  5
#define KEY0_PRIEMPTION_PRIORITY   2
#define KEY0_SUB_PRIORITY          1
#define KEY0_NVIC_GROUP            2

//PA15:KEY1
#define KEY1_GPIO                  GPIO_A
#define KEY1_PORT                  15
#define KEY1_PRIEMPTION_PRIORITY   2
#define KEY1_SUB_PRIORITY          0
#define KEY1_NVIC_GROUP            2

//PA0:WK_UP
#define WKUP_GPIO                   GPIO_A
#define WKUP_PORT                   0
#define WKUP_PRIEMPTION_PRIORITY    2
#define WKUP_SUB_PRIORITY           0
#define WKUP_NVIC_GROUP             2

//WWDT
#define WWDT_PRIEMPTION_PRIORITY    2
#define WWDT_SUB_PRIORITY           3
#define WWDT_NVIC_GROUP             2

//Timr3
#define TIM3_PRIEMPTION_PRIORITY    1
#define TIM3_SUB_PRIORITY           3
#define TIM3_NVIC_GROUP             2

#endif

