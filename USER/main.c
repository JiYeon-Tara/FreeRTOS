#include "test.h"
#include "ulog.h"

int main()
{
    board_peripheral_init();
    bsp_init();
    service_init();
    LOG_I("\r\n\r\nSTM32F103RCT6 Intialized.\r\n");

    while (1) {
        board_peripheral_test_loop();
        bsp_test_loop();
        component_test_loop();

        //exit
#if INT_KEY_ENABLE
        key_reset();
#endif
    }
    
    return 0;
}
    
