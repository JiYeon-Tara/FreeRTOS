/**
 * @file service_usart.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __SERVICE_USART_H__
#define __SERVICE_USART_H__
#include "usart.h"

typedef enum
{
    S_USART1 = 0,
    S_USART2,
    S_USART3,
    S_USART_MAX_NUM
} USART_NUM;

void uart1_print_recv_msg(USART_NUM usart);


#endif

