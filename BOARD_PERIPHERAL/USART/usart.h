#ifndef __USART_H__
#define __USART_H__

#include "board_config.h"

//USART1:组2，最低优先级， 抢占优先级 和 子优先级的值都是2
#if UART1_ENABLE
#define UART1_PRIEMPTION_PRIORITY  3
#define UART1_SUB_PRIORITY         3
#define UART1_NVIC_GROUP           2
#endif

#endif
