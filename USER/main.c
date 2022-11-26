#include "test.h"



int main()
{
    uint8_t tempArr[50];
    uint16_t tempNum;
    short temperature;

    bsp_init();
    printf("\r\n\r\n\r\n\r\n\r\nSTM32F103RCT6 Intialized.\r\n");
    service_init();

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

#if LCD_SCREEN_TEST
    lcd_screen_test();
#endif

#if RTC_TEST_ENABLE
    rtc_test();
#endif

#if DLPS_TEST_ENABLE
    dlps_test();
#endif

#if ADC_TEST_ENABLE
    ADC_test();
#endif

#if DMA_TEST_ENABLE
    dma_test();
#endif

    // loop
    while(1){
#if AT_CMD_TEST_ENABLE
        // receive from uart
        at_cmd_test();
#endif

#if RTC_ENABLE && LCD_SCREEN_ENABLE
        // sprintf((char*)tempArr, "Time:%d-%d-%d %d:%d:%d  Week:%d", calendar.w_year, calendar.w_month,calendar.w_date, \
        //                                             calendar.hour, calendar.min, calendar.sec, calendar.week);
        // LCD_ShowString(0, 32, 240, 16, 16, tempArr);
#endif
        
        // ADC
#if ADC_TEST_ENABLE
        tempNum = Get_Adc_Average(ADC_VAL_CH1, ADC_GET_TEST_COUNT);
        sprintf((char*)tempArr, "ADC:%2fV\n", adcVal2Voltage(tempNum));
        LCD_ShowString(0, 48, 240, 16, 16, (char*)tempArr);
        // temperature
        tempNum = Get_Adc_Average(ADC_MCU_TEMP_CHANNEL, ADC_GET_TEST_COUNT);
        sprintf((char*)tempArr, "CPU temperature:%2f degree\n", adcVal2Temper(tempNum));
        LCD_ShowString(0, 64, 240, 16, 16, (char*)tempArr);
#endif

#if DAC_TEST_ENABLE
        dac_test();
#endif

#if EEPROM_TEST_ENABLE
        eeprom_test();
#endif

#if FLASH_TEST_ENABLE
        flash_test();
#endif

#if REMOTE_CONTROL_TEST_ENABLE
        remote_test();
#endif

#if TEMP_TEST_ENABLE
        temperature = DS18B20_Get_Temp();
        printf("temperature:%d\n", temperature);
        delay_ms(1000);
#endif

#if INNER_FLASH_TEST_ENABLE
        inner_flash_test();
#endif

#if MALLOC_TEST_ENABLE
        memmang_test();
#endif
    }
    
    return 0;
}
    
