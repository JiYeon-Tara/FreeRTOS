#include "test.h"


int main()
{
    bsp_init();
    printf("\r\n\r\n\r\n\r\n\r\nSTM32F103RCT6 Intialized.\r\n");

#if LED_TEST_ENABLE
    led_test();
#endif

#if KEY_TEST_ENABLE
    key_test();
#endif

#if UART1_TEST_ENABLE
    usart_test();
#endif

#if WATCH_DOG_TEST_ENABLE
    watch_dog_test();
#endif

    while(1){
        
    }
    return;
}