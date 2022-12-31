#include "test.h"



int main()
{
    uint8_t tempArr[50];
    uint16_t tempNum;
    short temperature;

    bsp_init();
    service_init();
    printf("\r\n\r\nSTM32F103RCT6 Intialized.\r\n");

    // loop
    while(1){
#if AT_CMD_TEST_ENABLE
        at_cmd_test();
#endif

#if LED_TEST_ENABLE
        led_test();
#endif

#if LOOP_KEY_TEST_ENABLE
        key_test();
#endif

#if INT_KEY_TEST_ENABLE
#endif

#if IWATCH_DOG_TEST_ENABLE
        // feed watch doe
        IWDG_Feed();           
#endif

#if TIMER1_PWM_TEST_ENABLE
        timer1_pwm_test();
#endif

#if TIMER2_INPUT_TEST_ENABLE
        timer2_cap_test();
#endif

#if RTC_TEST_ENABLE
        rtc_update_test();
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
        // eeprom_test();
#endif

#if FLASH_TEST_ENABLE
        // flash_test();
#endif

#if TP_TEST_ENABLE
        // if(tp_dev.touchtype != 0xFF)
        //     LCD_ShowString(60, 130, 200, 16, 16, "Please start tp adjust process");
        if(tp_dev.touchtype & 0x80){ // 电容屏
            ctp_test();
        }
        else{
            rtp_test();
        }
#endif

#if REMOTE_CONTROL_TEST_ENABLE
        remote_test();
#endif

#if TEMP_TEST_ENABLE
        temperature = DS18B20_Get_Temp();
        printf("DS18B20 temperature:%d oC\n", temperature / 10);
        delay_ms(1000);
#endif

#if MOUSE_TEST_ENABLE
        mouse_test();
#endif

#if INNER_FLASH_TEST_ENABLE
        // inner_flash_test();
#endif

#if MEMORY_MANAGE_ENABLE
        // memmang_test(1);
        // memmang_test(0);
#endif
    }
    
    return 0;
}
    
