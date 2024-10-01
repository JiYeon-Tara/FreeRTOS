#ifndef __UTIL_H__
#define __UTIL_H__
#include "stm32f10x.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "sys.h"
#include "delay.h"
#include "ulog.h"
#include "board_config.h"
#include "bsp_config.h"
#include "service_config.h"
#include "gui_config.h"
#include "third_party_config.h"
/**************************************************************************
 * MACRO
 **************************************************************************/
// my implementation
// #define BIT(x)              (1 << (x))
#define SET_BIT_M(x)          (1 << (x)) // 1
#define RESET_BIT_M(x)        (~(1 << (x))) // 0   
#define CLEAR_BIT_M(x)        RESET_BIT_M(x)

// note: stm32f10x.h 中已经有这些宏定义
// #define SET_BIT(REG, BIT)     ((REG) |= (BIT))
// #define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
// #define READ_BIT(REG, BIT)    ((REG) & (BIT))
// #define CLEAR_REG(REG)        ((REG) = (0x0))
// #define WRITE_REG(REG, VAL)   ((REG) = (VAL))
// #define READ_REG(REG)         ((REG))
// #define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

#define UNUSED_PARAM(x)             ((void)x)

// ????
// TODO:
// #define CHECK_NOT_NULL(x)           (if (!x) {                  \
//                                         printf("null ptr");     \
//                                         return;                 \
//                                     })

/**************************************************************************
 * PUBLIC FUNCTION DECLARAION
 **************************************************************************/
void bit_oper_test(void);
u16 my_abs(u16 x1, u16 x2);
int reverse_byte(uint8_t *arr, uint32_t len);
int reverse_bit(uint8_t* arr, uint32_t len);

#endif
