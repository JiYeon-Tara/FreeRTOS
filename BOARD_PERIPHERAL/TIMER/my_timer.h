#ifndef __MY_TIMER_H__
#define __MY_TIMER_H__
#include "FreeRTOS.h"
#include "timers.h"
#include "usart.h"
/***************
 * EXTERNAL FUNCTION DECLARATION
 ****************/
void periodic_timer_cb(TimerHandle_t xTimer);


#endif // end __MY_TIMER_H__

