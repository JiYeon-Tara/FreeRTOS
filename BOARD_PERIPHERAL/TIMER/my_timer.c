#include "my_timer.h"

/**
 * @brief FreeRTOS 中 timer 的回调函数
 * 
 */
void periodic_timer_cb(TimerHandle_t xTimer)
{
    printf("%d timer cb invoked.\r\n");
}

