#ifndef __ULOG_H__
#define __ULOG_H__

#include "usart.h"

// 可变参数的宏
#define LOG_D(...)      printf(##__VA_ARGS__)

#endif

