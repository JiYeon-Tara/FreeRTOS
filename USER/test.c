/**
 * @file test.c
 * @author your name (you@domain.com)
 * @brief �����������蹦��ʵ��; �л���ͨ�� AT �������(service_at.c)
 * @version 0.1
 * @date 2022-11-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys.h"
#include "usart.h"		
#include "delay.h"	

#include "board_config.h"
#include "bsp_config.h"
#include "third_party_config.h"
#include "service_config.h"

#include "led.h" 
#include "key_stm.h" 
#include "service_usart.h"
#include "watch_dog_timer.h"
#include "timer.h"
#include "oled.h"
#include "ILI93xx.h"
#include "usmart.h"
#include "dma_stm.h"
#include "exti.h"
#include "at_cmd_parse.h"
#include "service_at.h"
#include "touch.h"
#include "mouse.h"
#include "sdcard.h"

#include "malloc.h"
#include "service_fs_api.h"
#include "exfuns.h"
#include "fattester.h"
#include "fs_test.h"

#include "ILI93xx.h"
#include "rtc.h"
#include "adc_stm.h"
#include "dac_stm.h"
#include "dma_stm.h"
#include "24cxx.h"
#include "w25q64.h"
#include "remote.h"
#include "ds18b20.h"
#include "stm_flash.h"
#include "touch.h"
#include "tp_test.h"

#include "malloc.h"
#include "timer.h"
#include "util.h"
#include "ulog.h"

#include "service_asm_test.h"

#include "gui/text/text.h"
// #include "gui/image/image.h"
// Program Size: Code=14966 RO-data=7166 RW-data=408 ZI-data=1848  
// FromELF: creating hex file...
// "..\OBJ\test.axf" - 0 Error(s), 13 Warning(s).
// Build Time Elapsed:  00:00:01

/**
 * @brief ������Ϣ����, ����ռ�� flash:14966 + 7166 = 22132 bytes
 * ռ�� SRAM ��С:1848 + 408 = 1856 bytes
 * 
 * code ��ʾ����ռ�� flash ��С
 * RO-data ��ʾ������ĳ���(flash)
 * RW-data ��ʾ��������Ѿ���ʼ���ı���
 * ZI-data ��ʾδ����ʼ���ı���
 * 
 * startup_stm32f10x_hd.s ��, �����˶�ջ(heap + stack) �ܴ�С 0x600 = 1536 bytes, 
 * 
 */


/*****************************************************************************
 * PRIVATE VARIABLES
 *****************************************************************************/
static uint8_t time_str[50] = "123456789";
#if LCD_SCREEN_ENABLE == 1
uint8_t random_string[50] = "Hello, world";
static uint8_t lcd_id[20];
const u8 char_size = 16;
u16 display_rect_width; // ��ʾ����Ŀ�
u16 display_rect_height; // ��ʾ����ĸ�
#endif

// #if DMA_TEST_ENABLE == 1
static uint8_t sendBuff[4196];
const uint8_t TEXT_TO_SEND[] = "hello, world";
extern int interrupt_enter_cnt;
#define TEXT_LENGTH 	(sizeof(TEXT_TO_SEND) - 1)	// Ҫ���͵����ݳ���
#define SEND_COUNT		300 // 300
// #endif
/*****************************************************************************
 * PRIVATE FUNCTIONS
 *****************************************************************************/
static void led_test(void)
{
    LED_Init();		  	 //��ʼ����LED���ӵ�Ӳ���ӿ�
    LED_Reset();
    delay_ms(5000);
    LED_Set();
    delay_ms(5000);
}

/**
 * @brief ������while(1) �е���
 * 
 */
static void key_test_loop(void)
{
#if LOOP_KEY_ENABLE	
    uint8_t val;

    // ������ while(1) �е���
    // while(1){
        val = KEY_Scan(SINGLE_SHOT_MODE);
        // printf("val:%d\n", val);
        switch(val){
            case KEY0_PRES:
                printf("val:%d key 0 pressed\n", val);
                break;
            case KEY1_PRES:
                printf("val:%d key 1 pressed\n", val);
                break;
            case WKUP_PRES:
                printf("val:%d, wkup pressed\n", val);
                break;
            default:
                break;
        }
    // }
#elif INT_KEY_ENABLE
    // EXTI_Init();
#endif
}

/**
 * @brief obsolete
 * 
 */
static void usart_test(void)
{
#if UART1_ENABLE
    uint8_t len;
    uint8_t ix = 0;

    uart1_init(72000000, 9600);	 //���ڳ�ʼ��Ϊ9600
    LED_Init();
    KEY_Init();

    while (1) {
        if(USART_RX_STA & UART_RX_COMPLETE){
            len = USART_RX_STA & UART_GET_RX_LEN;
            printf("recv len:%d\n", len);
            for(ix = 0; ix < len; ++ix){
                printf("%c", USART_RX_BUF[ix]);
            }
            printf("\r\n");
            USART_RX_STA = 0;
        }
    }
#endif
}

static void oled_screen_test(void)
{
    OLED_Clear();

    // OLED_ShowCHinese(0,0,0);//��
    // OLED_ShowCHinese(18,0,1);//��
    // OLED_ShowCHinese(36,0,2);//԰
    // OLED_ShowCHinese(54,0,3);//��
    // OLED_ShowCHinese(72,0,4);//��
    // OLED_ShowCHinese(90,0,5);//��
    // OLED_ShowCHinese(108,0,6);//��

    // OLED_ShowString(0,3,"1.3' OLED TEST");
    //OLED_ShowString(8,2,"ZHONGJINGYUAN");  
    //	OLED_ShowString(20,4,"2014/05/01");  
    // OLED_ShowString(0,6,"ASCII:");  
    // OLED_ShowString(63,6,"CODE:");

    // OLED_Clear();
    // OLED_ShowCHinese32x32(0, 0, 0); //��
    // OLED_ShowCHinese32x32(32, 0, 1); //��
    // OLED_ShowCHinese32x32(64, 0, 2); //԰

    // OLED_DrawBMP()
    return;
}

// while(1) �е���, ����ʱ��
static void rtc_update_test()
{
    sprintf((char*)time_str, "%d/%d/%d %d:%d:%d  %d", calendar.w_year, calendar.w_month,calendar.w_date, \
                                                   calendar.hour, calendar.min, calendar.sec, calendar.week);
#if OLED_SCREEN_ENABLE
    OLED_ShowString(0, 0, time);
#endif

#if LCD_SCREEN_ENABLE
    LCD_ShowString(0, 32, display_rect_width, display_rect_height, char_size, time_str);
#endif
}

/**
 * @brief ����Ҫ�ڴ�ѭ���� while(1) �е���
 * 
 */
static void at_cmd_test()
{
    // char *str = "AT^START"; // receive from uart
    // at_cmd_parse(str);
    uint8_t len;
    uint8_t ix = 0;

    // ���ʵ���߳���ִ�����, ������� while(1)
    // while(1){
        if(USART_RX_STA & UART_RX_COMPLETE){
            uint8_t bufPos = 0;
            len = USART_RX_STA & UART_GET_RX_LEN;
            // printf("recv len:%d\n", len);
            // for(ix = 0; ix < len; ++ix){
            // 	printf("%c", USART_RX_BUF[ix]);
            // }
            // printf("\r\n");
            

            if(USART_RX_BUF[0] == 'A' && USART_RX_BUF[1] == 'T'){
                // ���Ҫ���߳�, ���͵������̶߳���
                // thread_msg_send(&g_thread_manager, THREAD_MANAGER_ID_AT, &USART_RX_BUF[0], len);
                // ֱ�ӱ��ؽ���
                service_at_parse((char*)USART_RX_BUF);
                
            }
            else if(USART_RX_BUF[0] == 'F' && USART_RX_BUF[1] == 'I'){
                // ���Ҫ���߳�, ���͵������̶߳���
                // thread_msg_send(&g_thread_manager, THREAD_MANAGER_ID_FI, &USART_RX_BUF[0], len);
                // ֱ�ӱ��ؽ���
                // service_fi_parse((char*)USART_RX_BUF);
            }
            else{

            }
            // memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
            USART_RX_STA = 0;
            memset(USART_RX_BUF, 0, USART_REC_LEN);
        }
    // }
    return;
}

/**
 * @brief timer1 oputput pwm test
 *        100 : LED ����֮�������� Vav ��ʵ���Ȳ����̫��
 *        
 */
static void timer1_pwm_test(void)
{
    static uint8_t dir = 1;
    static uint16_t led0_pwm_val = 0; // ����ռ�ձ�

    delay_ms(10);
    if(dir == 1)
        ++led0_pwm_val;
    else
        --led0_pwm_val;
    
    if(led0_pwm_val > 100)  // Timer1 ��װ��ֵΪ 800����ʼ��С����
        dir = -1; // 
    if(led0_pwm_val == 0) // ��ʼ����
        dir = 1;
    TIM1_set_reload_val(led0_pwm_val);
    // printf("%d\r\n", led0_pwm_val);
}

/**
 * @brief while(1) �е���, �����������
 *        һ�� 1 ΢��, 1us
 *        0xFFFF Ϊ uint16_t �����ֵ, �Ƕ�ʱ��2��ʼ������װ��ֵ
 *        (1) ������������ͳ�Ʋ���ȷ:�����������ʱ��һֱ��һ������, ��֪��ʲôԭ�� - �� GPIO �ж�(����)����ʱ��, ����ͳ�Ƴ���
 * 
 */
static void timer2_cap_test(void)
{
    uint32_t temp;

    if(TIM2_CH1_CAPTURE_STA & TIM2_CAPTURE_COMPLETE){
        temp = (uint32_t)(TIM2_CH1_CAPTURE_STA & 0x3F); // ��ʱ����
        printf("pulse number:%ld, curr timer cnt:%d\r\n", temp, TIM2_CH1_CAPTURE_VAL);
        temp *= 0xFFFF; // uint16_t_max = 65535
        temp += TIM2_CH1_CAPTURE_VAL;
        printf("pulse width:%d us = %d s\r\n", temp, temp / 1000000);
        TIM2_CH1_CAPTURE_STA = 0; // start next capture
        TIM2_CH1_CAPTURE_VAL = 0;
    }

    return;
}

static void ADC_test_loop(void)
{
    uint8_t tempArr[50];
    uint16_t tempNum;
    short temperature;

    tempNum = Get_Adc_Average(ADC_VAL_CH1, ADC_GET_TEST_COUNT);
    sprintf((char*)tempArr, "PA1 Voltage:%2fV\n", adcVal2Voltage(tempNum));
    // TODO:
    // �������� while(1) ����ˢ��, Ӧ�÷ŵ���ʱ����ѭ����, ָ��ʱ��ˢһ��
    LCD_ShowString(0, 48, display_rect_width, display_rect_height, char_size, (char*)tempArr);
    // temperature
    tempNum = Get_Adc_Average(ADC_MCU_TEMP_CHANNEL, ADC_GET_TEST_COUNT);
    sprintf((char*)tempArr, "CPU Temp:%2f degree\n", adcVal2Temper(tempNum));
    LCD_ShowString(0, 64, display_rect_width, display_rect_height, char_size, (char*)tempArr);
}

// ֻ���� loop �е���
// �� DAC ������ǲ�
//^^^^^^^^^^^^^^^
static void dac_test_loop(void)
{
    static u16 dacVal = 0;
    static int dir = 1;

    if (dacVal <= 0) {
        dir = 1;  // increase
    } else if (dacVal >= DAC_OUTPUT_MAX_VOL) {
        dir = -1; // decrease
    }

    if (dir == 1) {
        dacVal += 100;
    } else if (dir == -1) {
        dacVal -= 100; // step = 0.1V
    }
    // LOG_D("dac-val:%d dir:%d", dacVal, dir);
    Dac1_Set_Vol(dacVal);
    delay_ms(1);
}


void dma_test(void)
{
    uint16_t t = 0, ix = 0;
    uint32_t sendNum;
    float progress = 0;	// ���������
    uint8_t progressStr[30];

    // DMA ����
    // UART1 TX - DMA1_Channel 4
    // ����Ϊ����1, �����ַλ�� USART1->DR �Ĵ����ĵ�ַ
    // �洢��Ϊ sendBuff, �洢����ַΪ�����׵�ַ
    // ����������:�� (TEXT_LENGTH+2) * 100 bytes
    // DMA_Config(DMA1_Channel4, (uint32_t)&USART1->DR, (uint32_t)sendBuff, (TEXT_LENGTH + 2) * SEND_COUNT);

    // ����Ҫ���͵����ݵ�������(�洢����ַ)
    for(ix = 0; ix < ((TEXT_LENGTH + 2) * SEND_COUNT); ix++){
        if(t >= TEXT_LENGTH){// ���뻻�з�
            sendBuff[ix++] = 0x0D;
            sendBuff[ix++] = 0x0A;
            t = 0;
        } else{
            sendBuff[ix] = TEXT_TO_SEND[t++];
        }
        // LOG_I("%d %d\n", ix, t);
    }

    // ��������һ��, �ʹ���һ������
    // ��ʼһ�� DMA ����
    // �ȴ� DMA �������, ��ʱ���ǿ�����һЩ��������(����Ҫ CPU ����)
    // ʵ��Ӧ����, DMA �����ڼ����ִ�����������

    LOG_I("DMA start transmit, send size:%d", ((TEXT_LENGTH + 2) * SEND_COUNT));
    // ����ʹ����??????
    USART1->CR3 |= (1 << 7); // ʹ�ܴ��ڵ� DMA ����

    DMA_Enable(DMA1_Channel4); // ��ʼ����һ��

    while (1) {
        if(DMA1->ISR & (1 << 13)){
            DMA1->IFCR |= 1 << 13; // ��� DMA1 ch4 �Ĵ�������жϱ�־
            LOG_I("DMA transfer completed.");
            break;
        }
        // ��ȡ��ǰ�������ݵİٷֱ�(�ܹ�Ҫ���� 100 ��)
        sendNum = DMA1_Channel4->CNDTR; // �õ���ǰ��ʣ����ٸ�����
        progress = 1 - (sendNum / ((TEXT_LENGTH + 2) * SEND_COUNT));
        progress *= 100; // ��ðٷֱ�
        sprintf(progressStr, "send:%d%%", progress);
        // ������ʹ��, ֱ��ʹ�� printf() ��ӡ���ݿ��ܻ��쳣
        // ���ٿ��Ե��� UART1 ���ڴ�ӡ
        // LOG_D("sendNum:%d progress:%d%%", sendNum, progress);
#if OLED_SCREEN_ENABLE == 1
        OLED_ShowString(50, 50, progressStr);
#elif LCD_SCREEN_ENABLE == 1
        LCD_ShowString(0, 100, display_rect_width, display_rect_height, char_size, progressStr);
#endif
    }
    LOG_I("DMA test finish\r\n");
}


void eeprom_test(void)
{
    typedef struct boot_mode {
        uint8_t factory_mode : 1;
        uint8_t app_mode : 1;
        uint8_t ota_mode : 1;
        uint8_t reserved : 5;
        uint8_t data;
    } boot_mode_t;

    boot_mode_t boot_mode;
    uint8_t count = 0;

#if defined(BASE_TEST)
    const uint8_t TEXT[] = "STM32_TEST_PROJECT";
    const uint8_t textSize = sizeof(TEXT); // �����β '\0'
    uint8_t buffer[256];

    while(AT24CXX_Check()){
        if(++count > 10){
            LOG_I("detect EEPROM failed\n");
            return;
        }
        LOG_I("Can't detect EEPROM:24C02, try again\n");
        delay_ms(500);
    }

    // EEPROM ���粻��ʧ�� 
    // Ϊ�˱�֤���ݵĿɿ���, ͨ������� CRC, ÿ�ζ�ȡǰУ��һ�� CRC
    delay_ms(1000);
    LOG_I("write:%s\n", TEXT);
    AT24CXX_Write(EEPROM_WRITE_START_ADDR, (uint8_t*)TEXT, textSize);	

    delay_ms(1000);
    AT24CXX_Read(EEPROM_WRITE_START_ADDR, buffer, textSize);
    LOG_I("read:%s\n", buffer);
#else
    while(AT24CXX_Check()){
        if(++count > 10){
            LOG_I("detect EEPROM failed\n");
            return;
        }
        LOG_I("Can't detect EEPROM:24C02, try again\n");
        delay_ms(500);
    }

    LOG_I("strcut size:%d\r\n", sizeof(boot_mode_t));

    boot_mode.app_mode = 1;
    LOG_I("boot mode before:%d %d %d\r\n", boot_mode.factory_mode, boot_mode.app_mode, boot_mode.ota_mode);
    AT24CXX_Write(EEPROM_WRITE_START_ADDR, (uint8_t*)&boot_mode, sizeof(boot_mode_t));

    memset(&boot_mode, 0, sizeof(boot_mode_t));
    LOG_I("boot mode inner:%d %d %d\r\n", boot_mode.factory_mode, boot_mode.app_mode, boot_mode.ota_mode);

    AT24CXX_Read(EEPROM_WRITE_START_ADDR, (uint8_t*)&boot_mode, sizeof(boot_mode_t));
    LOG_I("boot mode after:%d %d %d\r\n", boot_mode.factory_mode, boot_mode.app_mode, boot_mode.ota_mode);
#endif
    return;
}

/**
 * @brief Ƭ�� flash ����
 */
void external_flash_test(uint8_t dir)
{
    uint8_t count = 0;
    const uint8_t TEXT[] = "STM32_FLASH_OP_TEST";
    const uint8_t textSize = sizeof(TEXT); // �����β '\0'
    uint8_t buffer[256];
    uint8_t len;
    const uint32_t FLASH_SIZE = 8 * 1024 * 1024;	// falsh ��С:8M
    u8 unique_id[8] = {0};
    len = sizeof(unique_id);

    SPI_FLASH_TYPE = SPI_Flash_ReadID();//��ȡFLASH ID.
    LOG_I("external flash type:%#04X", SPI_FLASH_TYPE);
    if (SPI_FLASH_TYPE != W25Q64) {
        LOG_E("flash init failed");
        return;
    }

    SPI_Flash_ReadUniqueID((u8 *)unique_id, &len);
    LOG_HEX("external flash unique_id", unique_id, len);

    //TODO:
    /******************* flash ���ؾ���, ����һֱ��һ����ַ��д���� *******************/
    SPI_Flash_Write((uint8_t*)TEXT, W25Q64_START_ADDR, textSize);
    LOG_HEX("flash write", TEXT, textSize);

    memset(buffer, 0x00, sizeof(buffer));
    SPI_Flash_Read(buffer, W25Q64_START_ADDR, textSize);
    LOG_HEX("flash read", buffer, textSize);

    memset(buffer, 0x00, sizeof(buffer));
    SPI_Flash_Fast_Read(W25Q64_START_ADDR, buffer,  textSize);
    LOG_HEX("flash fast read", buffer, textSize);

    return;
}

static void remote_test(void)
{
    uint8_t keyVal = 0;
    char *str;

    while(1){
            keyVal = Remote_Scan();
            if(keyVal){
                switch(keyVal)
                {
                    case 0:str="ERROR";break;			   
                    case 162:str="POWER";break;	    
                    case 98:str="UP";break;	    
                    case 2:str="PLAY";break;		 
                    case 226:str="ALIENTEK";break;		  
                    case 194:str="RIGHT";break;	   
                    case 34:str="LEFT";break;		  
                    case 224:str="VOL-";break;		  
                    case 168:str="DOWN";break;		   
                    case 144:str="VOL+";break;		    
                    case 104:str="1";break;		  
                    case 152:str="2";break;	   
                    case 176:str="3";break;	    
                    case 48:str="4";break;		    
                    case 24:str="5";break;		    
                    case 122:str="6";break;		  
                    case 16:str="7";break;			   					
                    case 56:str="8";break;	 
                    case 90:str="9";break;
                    case 66:str="0";break;
                    case 82:str="DELETE";break;		
                    default:
                    break; 
                }
                printf("remote control value:%s\n", str);
        }
        else{
            delay_ms(10);
        }
    }
}

// Ƭ�� lfash ����
void inner_flash_test(uint8_t read_write_flag)
{
    const uint8_t TEXT[] = "This is a hello world test project.";
    const uint8_t textSize = sizeof(TEXT) + 1;	// '\0'
    uint8_t buffer[256] = {0};

    // flash д��Ҫע��:
    // (1) ����Ϊż��
    // (2) ��ַҪ���ڱ�����ռ�� flash ��С + 0x08000000, ��ҪС�� flash �ܴ�С, ���ܰѴ�����޸���
    // �ܴ�С:0x08000000 - 0x08000000 + 1024 * 256 = 08040000
    // Program Size: Code=53974 RO-data=8078 RW-data=420 ZI-data=3908  
    // Ŀǰ������:52K ����

    LOG_HEX("inner flash write:", TEXT, textSize);
    // uint8_t arr[] -> uint16_t*
    STMFLASH_Write(FLASH_SAVE_ADDR, (uint16_t*)TEXT, textSize);

    STMFLASH_Read(FLASH_SAVE_ADDR, (uint16_t*)buffer, textSize);
    LOG_HEX("inner flash read:", buffer, textSize);

    return;
}

/**
 * @brief ��ʾ��������ֵ
 * 
 * @param x �� LCD ����ʾ������λ��
 * @param y 
 * @param pos ����ֵ
 */
void Mouse_Show_Pos(u16 x, u16 y, short pos)
{
    if(pos<0)
    { 
#if LCD_SCREEN_ENABLE
        LCD_ShowChar(x, y, '-', 16, 0); //��ʾ����
#endif
        pos = -pos; //תΪ����
    }
    else 
#if LCD_SCREEN_ENABLE
        LCD_ShowChar(x, y, ' ', 16, 0);//ȥ������
#endif

#if LCD_SCREEN_ENABLE
    LCD_ShowNum(x + 8, y, pos, 5, 16); //��ʾֵ 
#endif
    return;
}

/**
 * @brief while(1) �е���
 * 
 */
static void mouse_test(void)
{
    static u8 errCnt = 0;

    if(!g_mouse_init_flag){
        return;
    }
    
    if(PS2_Status & (1 << 7)){ // 0x80
        Mouse_Data_Pro();
        printf("X:%d\r\nY:%d\r\nZ:%d\r\nflag:%02X\r\n", MouseX.x_pos, MouseX.y_pos, MouseX.z_pos, MouseX.bt_mask);
    }
    else if(PS2_Status & (1 << 6)){ // 0x40, ERRor
        errCnt++;
        PS2_Status = MOUSE;
    }
}

/**
 * @brief memory management test
 * 
 * @param flag 1 - malloc, 0 - free;
 */
void memmang_test(uint8_t flag)
{
    static void *pArr[10] = {0};
    static uint8_t pointerNum = 0;

    uint32_t *p = (uint32_t*)mymalloc(sizeof(uint32_t) * 500); // 2KB
    if (!p) {
        LOG_E("malloc failed\r\n");
        return;
    }
    mymemset(p, 0, sizeof(uint32_t) * 500);
    // save pointer
    pArr[pointerNum++] = p;
    LOG_I("memory alloc, address:0x%08X\r\n", p);
    LOG_I("memory used rate:%d%%\r\n", mem_perused());

    if(pointerNum <= 0){
        LOG_E("no memory to free\r\n");
        return;
    }
    p = pArr[--pointerNum];
    if(!p){
        LOG_E("no memory to free\r\n");
        return;
    }
    myfree(p);
    LOG_I("memory free, address:0x%08X\r\n", p);
    LOG_I("memory used rate:%d%%\r\n", mem_perused());

    return;
}

/**
 * @brief main loop �е���
 * ���°��� 0 �����ڴ�
 * ���°��� 1 �ͷ��ڴ�
 * ��ӡʹ����
 */
void mem_manage_test_loop(void)
{
    static void *pArr[10] = {0}; // ������ʮ��
    static int ptr_cnt = 0;
    uint8_t *ptr;

    switch (key_pressed()) {
        case KEY0_PRES: {
            if (ptr_cnt >= 10) {
                LOG_E("queue full");
                break;
            }
            ptr = mymalloc(1024);
            if (!ptr) {
                LOG_E("malloc failed");
                break;
            }
            pArr[ptr_cnt++] = ptr;
            snprintf(ptr, 1024, "ptr_cnt:%d", ptr_cnt);
            LOG_D("malloc ptr:%X data:%s", ptr, ptr);
            LOG_I("memory used rate:%d%%\r\n", mem_perused());
            break;
        }
        case KEY1_PRES: {
            if (ptr_cnt <= 0) {
                LOG_E("queue empty");
                break;
            }
            ptr = pArr[--ptr_cnt];
            LOG_D("free ptr:%X data:%s", ptr, ptr);
            myfree(ptr);
            LOG_I("memory used rate:%d%%\r\n", mem_perused());
            break;
        }
        default:
            break;
    }
}

/**
 * @brief ��ȡ SD card ָ������������
 * 
 * @param sect �ڼ�����
 * @param dir 0:read; 1-write
 */
void sdcard_read_write_sectorx_test(void)
{
    u8 *buff;
    u16 i;
    u32 sector_idx = 0;

    buff = (u8*)mymalloc(512);
    mymemset(buff, 0xA5, 512);

    if (SD_ReadDisk(buff, sector_idx, 1) == MSD_RESPONSE_SUCCESS) {
        LOG_HEX("sdcard sector0", buff, 512);
    } else {
        LOG_E("sdcard read error\r\n");
    }

    // mymemset(buff, 0xA5, 512);
    strcpy(buff, "start string:123456789");
    if(SD_WriteDisk(buff, sector_idx, 1) == MSD_RESPONSE_SUCCESS){
        LOG_I("sector write success\r\n");
    } else{
        LOG_E("sector write failed\r\n");
    }

    myfree(buff);

    return;
}
#if FATFS_ENABLE
/**
 * @brief ���� SDcard & ext flash ����ֱ��ʹ��ͬһ�� SPI1 ��ԭ��, 
 *        û�в���ϵͳ, ��ȫ���̲߳���, ֱ�Ӿ��Ƿ�ʱ����
 * 
 */
void fatfs_init(void)
{
    u8 ret;
    static FATFS fs0 = {0};
    static FATFS fs1 = {0};
    static uint8_t workspace0[_MAX_SS];
    static uint8_t workspace1[_MAX_SS];
    
    // ���� SDcard ��Ӧ�߼����� 0
    // Ĭ�� SDcard �Լ� �ڴ����ģ���Ѿ���ʼ���ɹ�
    // Q:sdcard �ϵ��ļ�ϵͳ��Ҫ��ʽ���Ժ�ſ����� STM32 �Ϲ��سɹ�???
    // A:��һ��ʹ����Ҫ���ļ�ϵͳ���и�ʽ��,��Ҫ�Ǵ��̷���
    ret = f_mount(&fs0, FATFS_LOGIC_VOLUME_0, 1); // �ļ�ϵͳ����ʧ��
    if(ret != FR_OK) {
        LOG_E("mount sdcard failed, ret:%d", ret);
        if (ret == FR_NO_FILESYSTEM) {
            LOG_E("try again");
            ret = f_mkfs(FATFS_LOGIC_VOLUME_0, 0, sizeof(workspace0));
            if (ret != FR_OK) {
                LOG_E("format failed ret:%d", ret);
                goto EXIT;
            }
            // ���¹���
            ret = f_mount(&fs0, FATFS_LOGIC_VOLUME_0, 1);
            if(ret != FR_OK) {
                LOG_E("mount sdcard failed, ret:%d", ret);
                goto EXIT;
            }
        } else {
            goto EXIT;
        }
    }
    LOG_I("FATFS volume %s mount success ret:%d", FATFS_LOGIC_VOLUME_0, ret);
    
    //TOOD:
    // ����������������������, W25Q64 �� SDcard ���� SPI1,
    // W25Q64 - �߼���1
    ret = f_mount(&fs1, FATFS_LOGIC_VOLUME_1, 1); // �ļ�ϵͳ����ʧ��
    if(ret != FR_OK) {
        LOG_E("mount ext flash W25Q64 failed, ret:%d", ret);
        if (ret == FR_NO_FILESYSTEM) {
            LOG_E("try again");
            ret = f_mkfs(FATFS_LOGIC_VOLUME_1, 0, sizeof(workspace1));
            if (ret != FR_OK) {
                LOG_E("format failed ret:%d", ret);
                goto EXIT;
            }
            // ���¹���
            ret = f_mount(&fs1, FATFS_LOGIC_VOLUME_1, 1);
            if(ret != FR_OK) {
                LOG_E("mount ext flash W25Q64 failed, ret:%d", ret);
                goto EXIT;
            }
        } else {
            goto EXIT;
        }
    }
    LOG_I("FATFS volume %s mount success ret:%d", FATFS_LOGIC_VOLUME_1, ret);

EXIT:
    return;
}
#if FATFS_TEST_ENABLE == 1
// FATFS �ṩ�Ľӿڲ���
static void fatfs_api_test1(const char *file_path)
{
    const uint8_t *path = (uint8_t*)file_path;
    uint8_t *data = "Hello, world\r\n";
    FIL *fp = NULL;
    FRESULT ret;
    u32 num;
    uint8_t *p_buff = NULL;

    fp = (FIL*)mymalloc(sizeof(FIL));
    if (!fp) {
        LOG_E("malloc failed");
        goto EXIT;
    }
    p_buff = mymalloc(512);
    if (!p_buff) {
        LOG_E("malloc failed");
        goto EXIT;
    }

    // ret = f_open(fp, (const TCHAR*)path, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
    ret = f_open(fp, (const TCHAR*)path, FA_READ | FA_WRITE);
    if (ret != FR_OK) {
        LOG_E("f_open failed path:%s ret:%d\r\n", path, ret);
        goto EXIT;
    }
    LOG_D("f_open %s success", path);

    // ret = f_write(fp, data, strlen(data) + 1, &num);
    // if (ret != FR_OK) {
    //     LOG_E("f_write failed path:%s\r\n", path);
    //     goto EXIT;
    // }
    // LOG_D("f_write success write len:%dBytes", num);

    ret = f_lseek(fp, 0);
    if (ret != FR_OK) {
        LOG_E("f_lseek failed path:%s\r\n", path);
        goto EXIT;
    }
    LOG_D("f_lseek success");

    ret = f_read(fp, p_buff, 512, &num);
    if (ret != FR_OK) {
        LOG_E("f_read failed path:%s\r\n", path);
        goto EXIT;
    }
    LOG_D("f_read success read len:%dBytes", num);
    LOG_HEX("f_read:", p_buff, num);

EXIT:
    if (fp) {
        ret = f_close(fp);
        if (ret != FR_OK) {
            LOG_E("f_close failed path:%s\r\n", path);
            return;
        }
        LOG_D("f_close success");
    }
    if (p_buff)
        myfree(p_buff);
    if (fp)
        myfree(fp);

    return;
}

/**
 * @brief �� FATFS �ӿڽ����˼򵥵ķ�װȻ����ԡ����о�û��Ҫ��ô��װ, ������Ϊ�˿����� AT �������
 *        
 * @param op 
 */
void exfuns_test(uint8_t op)
{
    uint8_t *path = "0:test.txt";
    uint8_t *data = "Hello, world\r\n";
    uint8_t readData[100];
    uint8_t *dirPath = "0:";
    u8 ret;

    switch(op){
        case 0:
        {
            // write test
            if(mf_open(path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK){
                printf("ERROR path:%s\r\n", path);
                return;
            }
            if(mf_write(data, strlen(data)) != FR_OK){
                printf("ERROR path:%s\r\n", path);
                return;
            }
            if(mf_close() != FR_OK) {
                printf("ERROR path:%s\r\n", path);
                return;
            }     
   
            // read test
            if(mf_open(path, FA_READ) != FR_OK){
                printf("ERROR path:%s\r\n", path);
                return;
            }
            if(mf_read(strlen(data)) != FR_OK){
                printf("ERROR path:%s\r\n", path);
                return;
            }
            if(mf_close() != FR_OK){
                printf("ERROR path:%s\r\n", path);
                return;
            }
        }
        break;
        case 1:
        {
            // �����򿪵�ǰĿ¼, �����ݹ������Ŀ¼
            if(ret = mf_opendir(dirPath) != FR_OK){
                printf("ERROR directory path:%s, errNo:%d\r\n", dirPath, ret);
            }
            mf_readdir();
            mf_closedir();
        }
        break;
        case 2:
        {
            if(ret = mf_scan_files(dirPath) != FR_OK){
                printf("ERROR directory path:%s, errNo:%d\r\n", dirPath, ret);
            }
        }
        break;
        case 3:
        {
            // �ѵ� FATFS �������Զ�д��ʽ���ļ� ??????????????
            if(ret = mf_open(path, FA_READ ) != FR_OK){
                printf("ERROR path:%s errNo:%d\r\n", path, ret);
                return;
            }
            if(ret = mf_read(strlen(data)) != FR_OK){
                printf("ERROR path:%s errNo:%d\r\n", path, ret);
                return;
            }

            if(ret = mf_close() != FR_OK){
                printf("ERROR path:%s, errNo:%d\r\n", path, ret);
                return;
            }
        }
        break;
        case 4:
        {
            // 0: ��ʾ�̷�
            // ��ʽ��
        //    if((ret = mf_fmkfs("0", FA_CREATE_ALWAYS, 1024)) != FR_OK){
        //         printf("create directory %s failed, errNo:%d\r\n", "0:TestDir", ret);
        //         return;
        //    }
        }
        break;
        case 5:
        {
            if((ret = mf_unlink(path)) != FR_OK){
                printf("delete %s failed, errNo:%d\r\n", path, ret);
                return;
           }
        }
        break;
        case 6:
        {
            if((ret = mf_mkdir("0:TestDir")) != FR_OK){
                printf("create dir %s failed, errNo:%d\r\n", "0:TestDir", ret);
                return;
            }
        }
        case 7:
        {
            char *filePath = "0:TestDir/abc.txt";
            if((ret = mf_open(filePath, FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK){
                printf("file %s create failed, errNo:%d", filePath, ret);
                return;
            }
            if((ret = mf_write(data, strlen(data))) != FR_OK){
                printf("file %s read failed, errNo:%d", filePath, ret);
                mf_close();
                return;
            }
        }
        break;
        break;
        default:
        break;
    }


    return;
}
#endif /* FS_TEST_ENABLE */
#endif /* FATFS_ENABLE */
/**
 * @brief GBK �ֿ����
 * 
 */
#if GUI_FONT_ENABLE
void gui_font_init(void)
{
#define ERASE_FLASH_FLAG 0
#if ERASE_FLASH_FLAG
    uint8_t random_arr[256] = {0};
    LOG_I("erase flash");
    // �������������, Ȼ����� flash �е��ֿ�����
    SPI_Flash_Write(random_arr, (4916 + 100) * 1024, sizeof(random_arr)); //�����ֿ���Ϣ
#endif
    LOG_I("gui_font_init");
    if (font_init() != 0) {
        LOG_E("get font info failed, update font");
        if (update_font() != 0) {
            LOG_E("update font failed");
            return;
        }
    }
}
#if GUI_FONT_TEST_ENABLE
void gui_font_test(void)
{
    LCD_Show_Str(0, 200, display_rect_width, display_rect_height, "China No 1.", char_size, 0);
    LCD_Show_Str(0, 220, display_rect_width, display_rect_height, "�����Ǹ����", char_size, 0);

    // lcd_show_unicode_str(0, 220, display_rect_width, display_rect_height, L"�����Ǹ����", char_size, 0);
}
#endif /* GUI_FONT_TEST_ENABLE */
#endif /* GUI_FONT_ENABLE */

#if GUI_IMAGE_ENABLE
//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
static u16 pic_get_tnum(u8 *path)
{	  
    u8 res;
    u16 rval = 0;
    DIR tdir; //��ʱĿ¼
    FILINFO tfileinfo; //��ʱ�ļ���Ϣ	
    u8 *fn;	 			 			   			     

    tfileinfo.lfsize = _MAX_LFN*2+1; //���ļ�����󳤶�
    tfileinfo.lfname = mymalloc(tfileinfo.lfsize); //Ϊ���ļ������������ڴ�
    if (!tfileinfo.lfname) {
        LOG_E("malloc failed");
        return rval;
    }

    res = f_opendir(&tdir,(const TCHAR*)path); //��Ŀ¼
    if (res != FR_OK) {
        myfree(tfileinfo.lfname);
        return rval;
    }
    while (1) { //��ѯ�ܵ���Ч�ļ���
        res = f_readdir(&tdir,&tfileinfo); //��ȡĿ¼�µ�һ���ļ�
        if(res != FR_OK || tfileinfo.fname[0] == 0)
            break;	//������/��ĩβ��,�˳�		  
        fn = (u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
        res = f_typetell(fn);	
        if((res & 0XF0) == 0X50) {//ȡ����λ,�����ǲ���ͼƬ�ļ�	
            rval++;//��Ч�ļ�������1
        }	    
    }
    // TODO:
    // close dir ?
    LOG_I("%s pic num:%d", path, rval);

    return rval;
}

/**
 * @brief ����ʵ�ֲ�����:1.ֱ�ӵ��� fatfs �ڲ��ӿ�, 2.Ϊʲô��ֱ�ӱ����ļ����� 
 *  �����ļ������ļ�������
 * 
 * @param dir_path 
 */
static void gui_get_image_info_list(const char *dir_path)
{
    DIR picdir;
    FILINFO picfileinfo;
    u16 totoal_pic_num;
	u8 *pname; //��·�����ļ���
	u16 *picindextbl;	//ͼƬ������ 

    if (f_opendir(&picdir, (const TCHAR*)dir_path) != FR_OK) { // ��ͼƬ�ļ���
        LOG_E("%s open failed", dir_path);
        return;
    }

    totoal_pic_num = pic_get_tnum((u8*)dir_path); //�õ�����Ч�ļ���
    if (totoal_pic_num == 0) {
        LOG_E("total pic num:%d", totoal_pic_num);
        return;
    }

    // TODO:
  	// picfileinfo.lfsize=_MAX_LFN*2+1;						//���ļ�����󳤶�
	// picfileinfo.lfname=mymalloc(picfileinfo.lfsize); //Ϊ���ļ������������ڴ�
 	// pname=mymalloc(picfileinfo.lfsize); //Ϊ��·�����ļ��������ڴ�
 	// picindextbl=mymalloc(2*totoal_pic_num); //����2*totpicnum���ֽڵ��ڴ�,���ڴ��ͼƬ����
 	// if (picfileinfo.lfname == NULL || pname == NULL || picindextbl==NULL) {//�ڴ�������
 	// 	LOG_E("malloc failed");
    //     return;
	// }
    
    //
}

void gui_image_init(void)
{
    LOG_I("into");
    piclib_init();
    gui_get_image_info_list("0:PICTURE");
}
#if GUI_IMAGE_TEST_ENABLE
void gui_image_test(void)
{
    ai_load_picfile("0:PICTURE/person_gif.gif",0,0,lcddev.width,lcddev.height,1);//��ʾͼƬ 
}
#endif /* GUI_IMAGE_TEST_ENABLE */
#endif /* GUI_IMAGE_ENABLE */
/*****************************************************************************
 * PUBLIC FUNCTIONS
 *****************************************************************************/
void board_peripheral_init(void)
{
    Stm32_Clock_Init(9); //ϵͳʱ������
    NVIC_PriorityGroupConfig(DEFAULT_NVIC_GROUP);
    delay_init(72000000); //��ʱ��ʼ��
    delay_ms(100);

#if UART1_ENABLE
    // ��ʼ��֮��ſ���ʹ�� printf()
    uart1_init(72000000, 115200);	 //���ڳ�ʼ��Ϊ9600
    LOG_I("***************************************");
    LOG_I("*App start");
    LOG_I("***************************************");
    LOG_I("uart init\r\n");
#endif

#if NVIC_TEST == 1
    NVIC_Get_Chip_Support_Priority_Bit_Num();
#endif

#if DLPS_ENABLE == 1
    WKUP_Init();
#endif

#if IWATCH_DOG_ENABLE
    IWDG_Init(4, 625); // 100:64��Ƶ, ReloadValue:625, 1000ms
#elif WWATCH_DOG_ENABLE
    WWDG_Init(WWDG_CNT, WWDG_UPPER_CNT, WWDG_CLK_8_PRESCALLER); // ������
#endif

#if TIMER1_ENABLE
    TIM1_PWM_Init(899, 0); // ����Ƶ�� fpwm = 72 MHz / (899 + 1) = 0.08 MHz
#endif

#if TIMER2_ENABLE
    // freq: 72 M / ((0xFFFF + 1) * 72), ���� 1M Hz Ƶ��? ����ʱ�侫�� 1 us
    // TIM2_CNT ÿ +1 ���� 1us, ���ͳ�� 65535 * 1 = 65.5ms
    // ����ͳ�Ƶĳ�ʱ����:0011 1111 0x3F * 65 = 4095ms = 4s
    TIM2_INPUT_Init(0xFFFF, 72 - 1); 
#endif

#if TIMER3_ENABLE
    // TIM3_Int_Init(4999, 8999); // ��ʱ��Ƶ�� 9KHz, 0.625s ����
    TIM3_Int_Init(4999 * 10, 8999); // ��ʱ��Ƶ�� 9KHz, 6.25s ����
#endif

#if ADC_ENABLE
    Adc_Init();
#endif

#if DAC_ENABLE
    Dac1_Init();
#endif

#if RTC_ENABLE
    setRealTimeRtc();
    RTC_Init();
    sprintf((char*)time_str, "%d-%d-%d %d:%d:%d  %d", calendar.w_year, calendar.w_month,calendar.w_date,
                                                   calendar.hour, calendar.min, calendar.sec, calendar.week);
#endif

#if DMA_ENABLE
    // USART1 TX - DMA1_Channel4
    // �����ַ:(u32)USART1->DR, Ҳ����ֱ��д�Ĵ�����ַ
    // �洢����ַ:ȫ�ֱ�����ַ, �洢������ģʽ
    // ���ʹ�С:һ�δ�С * ����
    DMA_Config(DMA1_Channel4, (u32)&USART1->DR, (u32)sendBuff, (TEXT_LENGTH + 2) * SEND_COUNT);
#if DMA_TEST_ENABLE
    dma_test(); 
#endif // DMA_TEST_ENABLE
    LOG_I("interrupt_enter_cnt:%d", interrupt_enter_cnt);
#endif // DMA_ENABLE

#if INNER_FLASH_ENABLE
    // Ƭ�� falsh ����Ҫ��ʼ��ֱ��ʹ�ü���
#if INNER_FLASH_TEST_ENABLE
    inner_flash_test(1);
#endif
#endif

#if USB_ENABLE
    LOG_I("usb init");
 	USB_Port_Set(0); 	//USB??????
	delay_ms(300);
   	USB_Port_Set(1);	//USB????????
	//USB????
 	USB_Interrupts_Config();    
 	Set_USBClock();   
 	USB_Init();
#endif
}

void bsp_init(void)
{
    int count = 0;
    int ret;
    u32 sd_sect_size;
    u8 arr[1] = {0x5A};

#if LED_ENABLE
    LED_Init();
#endif

#if KEY_ENABLE
    KEY_Init();
#endif

#if LCD_SCREEN_ENABLE
    LCD_Init();
    display_rect_width = lcddev.width;
    display_rect_height = char_size;
    sprintf((char*)lcd_id, "LCD ID:0x%04X", lcddev.id);
    LOG_I("%s\r\n", lcd_id);

#if LCD_SCREEN_TEST_ENABLE == 1
    LCD_ShowString(0, 0, display_rect_width, display_rect_height, char_size, lcd_id);
    LCD_ShowString(0, 16, display_rect_width, display_rect_height, char_size, random_string);
    // LCD_ShowString(0, 100, display_rect_width, display_rect_height, char_size, "This is a screen display test");
#if 0
    LCD_ShowCharSong(0, 200, 'A', 12, 0);
    LCD_ShowCharSong(0, 220, 'A', 16, 0);
    LCD_ShowCharSong(0, 240, 'A', 24, 0);
#endif
#if RTC_ENABLE == 1
    RTC_Get();
    snprintf(time_str, sizeof(time_str), "%d-%d-%d %d:%d:%d  %d", calendar.w_year, calendar.w_month,
        calendar.w_date, calendar.hour, calendar.min, calendar.sec, calendar.week);
#endif
#endif /* LCD_SCREEN_TEST_ENABLE */
#endif /* LCD_SCREEN_ENABLE */

#if TP_ENABLE
    TP_Init();
#endif

#if OLED_SCREEN_ENABLE
    OLED_Init();
    // OLED_ShowString(0, 0, time);
#if OLED_SCREEN_TEST_ENABLE
    oled_screen_test();
#endif
#endif

#if EEPROM_ENABLE
    AT24CXX_Init();
    AT24CXX_Check();
#if EEPROM_TEST_ENABLE
    // AT24CXX_read_write_test();
    AT24CXX_CurrAddrRead_test();
#endif
#endif /* EEPROM_ENABLE */

#if FLASH_ENABLE
    SPI_Flash_Init();
#if FLASH_TEST_ENABLE
    external_flash_test(1);
#endif
#endif

#if REMOTE_CONTROL_ENABLE
    Remote_Init();
#endif

#if MOUSE_ENABLE == 1
    while((ret = Init_Mouse()) && count < 5){
        printf("Mouse init\r\n");
        count++;
    }
    if(count < 5){
        printf("mouse init success, mouse:%d\r\n", MOUSE_ID);
        g_mouse_init_flag = 1;
    }
    else{
        printf("mouse init failed, err code:%d\r\n", ret);
        g_mouse_init_flag = 0;
    }
    count = 0;
#endif

#if SD_CARD_ENABLE == 1
    while(ret = SD_Initialize()) { // ��ⲻ�� SD ��
        LOG_E("sdcard init failed\r\n");
    }
    if(!ret){
        LOG_E("SD card init success\r\n");
        sd_sect_size = SD_GetSectorCount(); // ��ȡ������
        LOG_E("SD card sector number:%d\r\n", sd_sect_size);
    }
#if SD_CARD_TEST_ENABLE
    // LOG_HEX("arr:", arr, 1);
    // reverse_byte(arr, 1);
    // LOG_HEX("reverse byte arr:", arr, 1);
    // reverse_bit(arr, 1);
    // LOG_HEX("reverse bit arr:", arr, 1);
    sdcard_read_write_sectorx_test();
#endif
#endif /* SD_CARD_ENABLE */
    
#if TEMP_ENABLE
    while(DS18B20_Init() && ++count < 5) {
        LOG_E("DS18B20 does not exist\n");
        delay_ms(500);
    }
#endif /* TEMP_ENABLE */
}

void service_init(void)
{
    u32 total, free;
    u8 ret;

#if MEMORY_MANAGE_ENABLE
    mem_init();
#if MEMORY_MANAGE_ENABLE
    memmang_test(1);
#endif
#endif /* MEMORY_MANAGE_ENABLE */

#if FS_ENABLE
#if FATFS_ENABLE
    fatfs_init();
    // TODO:
    // �Ƴ���ʱ����Ҫȡ������
    // f_unmount("0:");
    // f_unmount("1:");
#if FATFS_TEST_ENABLE
    fs_op_test_framework();
    // fatfs_api_test1("0:/test_sdcard.txt");
    // fatfs_api_test1("1:/test_flash.txt");

    // �Լ��򵥷�װ�Ĳ��Ժ���
    // if((ret = exfuns_init()) != 0){
    //     LOG_E("FATFS init failed, ret:%d", ret);
    //     goto EXIT;
    // }
    // exfuns_test();
    // while(exf_getfree(FATFS_LOGIC_VOLUME_0, &total, &free)){
    //     LOG_E("get sdcard free fail");
    // }
    // LOG_I("disk capacity, total:%dKB(%dMB), free:%dKB(%dMB)", 
    //         total, total/1024, free, free/1024);
#endif /* FATFS_TEST_ENABLE */

#if YAFFS_ENABLE
#error "does not implement"
#endif /* YAFFS_ENABLE */

#if FS_API_ENABLE
    // �ļ�ϵͳ API ֧��
    file_system_ctx_init();
#if FS_API_TEST_ENABLE

#endif /* FS_API_TEST_ENABLE */
#endif /* FS_API_ENABLE */

#endif /* FATFS_ENABLE */
#endif /* FS_ENABLE */

#if SERVICE_ASM_TEST == 1
    // service_asm_test_load_global_val();
    // nested_fun_call_fun1(100, 200, 300);
    // null_ptr_test(NULL, 1);
    div_0_test();
#endif

    return;
EXIT:
    LOG_E("ERROR");
    return;
}

void gui_init(void)
{
    LOG_I("enter");
#if GUI_FONT_ENABLE
    gui_font_init();
#if GUI_FONT_TEST_ENABLE
    gui_font_test();
#endif
#endif

#if GUI_IMAGE_ENABLE
    gui_image_init();
#if GUI_IMAGE_TEST_ENABLE
    gui_image_test();
#endif
#endif
    LOG_I("exit");
}

/**
 * @brief while(1) �е���
 * 
 */
void board_peripheral_test_loop(void)
{
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
        
#if ADC_TEST_ENABLE == 1
    ADC_test_loop();
#endif

#if DAC_TEST_ENABLE
    dac_test_loop();
#endif
}

/**
 * @brief while(1) �е���
 * 
 */
void bsp_test_loop(void)
{
    short temperature;
    u8 temp_str[128] = {0}; 

#if LED_TEST_ENBLE
    led_test_loop();
#endif

#if LOOP_KEY_TEST_ENABLE
    key_test_loop();
#endif

#if INT_KEY_TEST_ENABLE
#endif

#if TP_ENABLE
    tp_test_loop();
#endif

#if TEMP_TEST_ENABLE
    temperature = DS18B20_Get_Temp();
    snprintf(temp_str, sizeof(temp_str), "External Temperature:%f C", (float)temperature/10);
    LOG_D("%s", temp_str);
#if LCD_SCREEN_ENABLE
    LCD_ShowString(0, 150, display_rect_width, display_rect_height, char_size, temp_str);
#endif /* LCD_SCREEN_ENABLE */
    delay_ms(1000);
#endif /* TEMP_TEST_ENABLE */

}

/**
 * @brief �������� loop
 *        while(1) �е���
 * 
 */
void component_test_loop(void)
{
    u8 arr[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '10'};

#if MEMORY_MANAGE_TEST_ENABLE
    mem_manage_test_loop();
#endif

#if AT_CMD_TEST_ENABLE
    at_cmd_test();
#endif

#if 0
    LOG_HEX("arr", arr, 10);
#endif
}

void gui_test_loop(void)
{

}

