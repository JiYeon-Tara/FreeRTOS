/**
 * @file test.c
 * @author your name (you@domain.com)
 * @brief 测试所有外设功能实现; 切换到通过 AT 命令测试(service_at.c)
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

#include "led.h" 
#include "key_stm.h" 
#include "service_usart.h"
#include "watch_dog_timer.h"
#include "timer.h"
#include "oled.h"
#include "LCD.h"
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


// Program Size: Code=14966 RO-data=7166 RW-data=408 ZI-data=1848  
// FromELF: creating hex file...
// "..\OBJ\test.axf" - 0 Error(s), 13 Warning(s).
// Build Time Elapsed:  00:00:01
/**
 * @brief 编译信息解释, 代码占用 flash:14966 + 7166 = 22132 bytes
 * 占用 SRAM 大小:1848 + 408 = 1856 bytes
 * 
 * code 表示程序占用 flash 大小
 * RO-data 表示程序定义的常量(flash)
 * RW-data 表示程序定义的已经初始化的变量
 * ZI-data 表示未被初始化的变量
 * 
 * startup_stm32f10x_hd.s 中, 定义了堆栈(heap + stack) 总大小 0x600 = 1536 bytes, 
 * 
 */

/***************
 * VARIABLES
 ***************/
uint8_t time[50];
uint8_t lcd_id[20];

#if DMA_TEST_ENABLE
static uint8_t sendBuff[4196];
const uint8_t TEXT_TO_SEND[] = "hello, world";
#define TEXT_LENGTH 	(sizeof(TEXT_TO_SEND) - 1)	// 要发送的数据长度
#define SEND_COUNT		300
#endif

void bsp_init()
{
    uint8_t temp[50];
    uint8_t cpuTemp = 50;
    uint16_t year = 2022, month = 9, day = 1, hour = 8, min = 0, sec = 0; 
    uint8_t count = 0;
    uint8_t ret;
    uint32_t sd_sect_size;

    Stm32_Clock_Init(9); 		 //系统时钟设置
    delay_init(72000000);	     //延时初始化
    delay_ms(100);
#if USART1_ENABLE
    // 初始化之后才可以使用 printf()
    uart1_init(72000000, 115200);	 //串口初始化为9600
    printf("\r\n\r\n\r\n\r\n\r\nsystem restart\r\n");
    printf("uart init\r\n");
#endif

#if LED_ENABLE
    LED_Init();
#endif

#if KEY_ENABLE
    KEY_Init();
#endif

#if IWATCH_DOG_ENABLE
    IWDG_Init(4, 625);
#elif WWATCH_DOT_TEST_ENABLE
    WWDG_Init(WWDG_CNT, WWDG_UPPER_CNT, SPLIT_FREQ); // 计数器
#endif

#if TIMER1_ENABLE
    TIM1_PWM_Init(899, 0); // 不分频， fpwm = 72 MHz / (899 + 1) = 0.08 MHz
#endif

#if TIMER2_ENABLE
    // freq: 72 M / ((0xFFFF + 1) * 72), 这是 1M Hz 频率? 采样时间精度 1 us
    // TIM2_CNT 每 +1 等于 1us, 最多统计 65535 * 1 = 65.5ms
    // 加上统计的超时次数:0011 1111 0x3F * 65 = 4095ms = 4s
    TIM2_INPUT_Init(0xFFFF, 72 - 1); 
#endif

#if TIMER3_ENABLE
    TIM3_Int_Init(4999, 8999); // 定时器频率 9KHz, 计数到
#endif

#if ADC_ENABLE
    Adc_Init();
#endif

#if DAC_ENABLE
    Dac1_Init();
#endif

#if RTC_ENABLE
    RTC_Init();
    sprintf((char*)time, "%d/%d/%d %d:%d:%d  %d", calendar.w_year, calendar.w_month,calendar.w_date, \
                                                   calendar.hour, calendar.min, calendar.sec, calendar.week);
#endif

#if OLED_SCREEN_ENABLE
    OLED_Init();
    OLED_ShowString(0, 0, time);
#if OLED_SCREEN_TEST_ENABLE
    oled_screen_test();
#endif // OLED_SCREEN_TEST_ENABLE
#endif // OLED_SCREEN_ENABLE

#if LCD_SCREEN_ENABLE
    LCD_Init();
    sprintf((char*)lcd_id, "LCD ID:0x%04X", lcddev.id);
    printf("%s\r\n", lcd_id);
#endif

#if DMA_ENABLE
#if DMA_TEST_ENABLE
    // USART1 TX - DMA1_Channel4
    // 外设地址:(u32)USART1->DR, 也可以直接写寄存器地址
    // 存储器地址:全局变量地址, 存储器增量模式
    // 发送大小:一次大小 * 次数
    DMA_Config(DMA1_Channel4, (u32)&USART1->DR, (u32)sendBuff, (TEXT_LENGTH + 2) * SEND_COUNT);
#endif // DMA_TEST_ENABLE
#endif // DMA_ENABLE

#if EEPROM_ENABLE
    AT24CXX_Init();
    AT24CXX_Check();
#if EEPROM_TEST_ENABLE
    AT24CXX_read_write_test();
#endif
#endif

#if FLASH_ENABLE
    SPI_Flash_Init();
#if FLASH_TEST_ENABLE
    flash_read_write_test();
#endif
#endif

#if REMOTE_CONTROL_ENABLE
    Remote_Init();
#endif

#if TEMP_ENABLE
    while(DS18B20_Init() && ++count < 5)
    {
        printf("DS18B20 does not exist\n");
        delay_ms(500);
    }
#endif

#if INNER_FLASH_ENABLE
        // 片内 falsh 不需要初始化直接使用即可
#endif

#if LCD_SCREEN_TEST_ENABLE
    LCD_ShowString(0, 0, 240, 16, 16, lcd_id);
    LCD_ShowString(0, 16, 240, 16, 16, temp);
    // LCD_ShowString(0, 32, 240, 16, 16, time);
#endif

#if TP_ENABLE
    TP_Init();
#endif

#if MOUSE_ENABLE
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

#if SD_CARD_ENABLE
    while(ret = SD_Initialize()){ // 检测不到 SD 卡
        printf("sdcard init failed\r\n");
    }
    if(!ret){
        printf("SD card init success\r\n");
        sd_sect_size = SD_GetSectorCount(); // 获取扇区数
        printf("SD card sector number:%d\r\n", sd_sect_size);
    }
#endif
}


void service_init()
{
    u32 total, free;
    u8 ret;

#if MEMORY_MANAGE_ENABLE
    mem_init();
#endif

#if FATFS_ENABLE
    // 默认 SDcard 以及 内存管理模块已经初始化成功
    // sdcard 上的文件系统需要格式化以后才可以在 STM32 上挂载成功???
    // 使用自己写过的 SDcard 挂载文件系统会失败
    if((ret = exfuns_init()) != 0){
        printf("FATFS init failed, ret:%d\r\n", ret);
        goto EXIT;
    }

    // 文件系统挂载失败
    if((ret = f_mount(fs[0], FS_FILE_SYSTEM_NAME, 1)) != FR_OK) { // 挂载 SDcard
    // if((ret = f_mount(fs[0], "0:", 1)) != FR_OK) { // 挂载 SDcard
        printf("mount sdcard failed, ret:%d\r\n", ret);
        goto EXIT;
    }
    printf("FS:%s mount success\r\n", FS_FILE_SYSTEM_NAME);
    if((ret = f_mount(fs[1], "1", 1)) != FR_OK){ // 挂载外部 flash
        printf("mount external flash failed, ret:%d\r\n", ret);
        goto EXIT;
    }
    printf("FS:%s mount success\r\n", "1");

    while(exf_getfree(FS_FILE_SYSTEM_NAME, &total, &free)){
        printf("get sdcard free fail\r\n");
    }
    printf("disk capacity, total:%dKB(%dMB), free:%dKB(%dMB)\r\n", total, total/1024, free, free/1024);
    // 挂载两个物理磁盘有问题
    // 目前先只测试 flash
    while(exf_getfree("1", &total, &free)){
        printf("get flash free fail\r\n");
    }
    printf("external flash capacity, total:%dKB(%dMB), free:%dKB(%dMB)\r\n", total, total/1024, free, free/1024);


#endif

#if FS_API_ENABLE
    // 文件系统 API 支持
    file_system_ctx_init();
#endif
    return;
EXIT:
    return;
}


void led_test()
{
    LED_Init();		  	 //初始化与LED连接的硬件接口
    LED_Reset();
    delay_ms(5000);
    LED_Set();
    delay_ms(5000);
}

/**
 * @brief 必须在while(1) 中调用
 * 
 */
void key_test()
{
#if LOOP_KEY_ENABLE	
    uint8_t val;

    // 必须在 while(1) 中调用
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
    EXTI_Init();
#endif
}

void usart_test()
{
#if USART1_ENABLE
    uint8_t len;
    uint8_t ix = 0;

    uart1_init(72000000, 9600);	 //串口初始化为9600
    LED_Init();
    KEY_Init();

    while(1){
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

void watch_dog_test()
{
#if IWATCH_DOG_TEST_ENABLE
    IWDG_Init(4, 625); // 溢出时间为 1s
#elif WWATCH_DOG_TEST_ENABLE
    
#endif
}

void oled_screen_test()
{
    OLED_Clear();
    // OLED_ShowCHinese(0,0,0);//中
    // OLED_ShowCHinese(18,0,1);//景
    // OLED_ShowCHinese(36,0,2);//园
    // OLED_ShowCHinese(54,0,3);//电
    // OLED_ShowCHinese(72,0,4);//子
    // OLED_ShowCHinese(90,0,5);//科
    // OLED_ShowCHinese(108,0,6);//技
    // OLED_ShowString(0,3,"1.3' OLED TEST");
    // //OLED_ShowString(8,2,"ZHONGJINGYUAN");  
    // //	OLED_ShowString(20,4,"2014/05/01");  
    // OLED_ShowString(0,6,"ASCII:");  
    // OLED_ShowString(63,6,"CODE:");  
    // OLED_DrawBMP()
    return;
}

void lcd_screen_test()
{
    // uint8_t lcd_id[20];

    // LCD_Init();
    // printf("LCD_Init\n");
    // sprintf((char*)lcd_id, "LCD ID:%04X", lcddev.id);

    // LCD_ShowString(0, 0, 240, 16, 16, lcd_id);
    // LCD_Draw_Circle(20, 20, 10);
    // LCD_DrawLine(40, 40, 80, 80);	

}

void rtc_test()
{
    setRealTimeRtc();
    RTC_Init();
    RTC_Set(calendar.w_year, calendar.w_month, calendar.w_date, calendar.hour, calendar.min, calendar.sec);
    while(1){
        sprintf((char*)time, "%d/%d/%d %d:%d:%d  %d", calendar.w_year, calendar.w_month,calendar.w_date, \
                                                calendar.hour, calendar.min, calendar.sec, calendar.week);
        delay_ms(5000);
    }
}

// while(1) 中调用, 更新时间
void rtc_update_test()
{
    sprintf((char*)time, "%d/%d/%d %d:%d:%d  %d", calendar.w_year, calendar.w_month,calendar.w_date, \
                                                   calendar.hour, calendar.min, calendar.sec, calendar.week);
#if OLED_SCREEN_ENABLE
    OLED_ShowString(0, 0, time);
#endif

#if LCD_SCREEN_ENABLE
    LCD_ShowString(0, 32, 240, 16, 16, time);
#endif
}

/**
 * @brief 必须要在大循环的 while(1) 中调用
 * 
 */
void at_cmd_test()
{
    // char *str = "AT^START"; // receive from uart
    // at_cmd_parse(str);
    uint8_t len;
    uint8_t ix = 0;

    // 如果实在线程中执行这个, 则可以用 while(1)
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
                // 如果要多线程, 则发送到解析线程对列
                // thread_msg_send(&g_thread_manager, THREAD_MANAGER_ID_AT, &USART_RX_BUF[0], len);
                // 直接本地解析
                service_at_parse((char*)USART_RX_BUF);
                
            }
            else if(USART_RX_BUF[0] == 'F' && USART_RX_BUF[1] == 'I'){
                // 如果要多线程, 则发送到解析线程对列
                // thread_msg_send(&g_thread_manager, THREAD_MANAGER_ID_FI, &USART_RX_BUF[0], len);
                // 直接本地解析
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
 *        100 : LED 点亮之后再增加 Vav 其实亮度差别不是太大
 *        
 */
void timer1_pwm_test()
{
    static uint8_t dir = 1;
    static uint16_t led0_pwm_val = 0; // 控制占空比

    delay_ms(10);
    if(dir == 1)
        ++led0_pwm_val;
    else
        --led0_pwm_val;
    
    if(led0_pwm_val > 100)  // Timer1 重装载值为 800，开始减小方向
        dir = -1; // 
    if(led0_pwm_val == 0) // 开始增大
        dir = 1;
    TIM1_set_reload_val(led0_pwm_val);
    // printf("%d\r\n", led0_pwm_val);
}

/**
 * @brief while(1) 中调用, 捕获脉冲宽度
 *        一次 1 微秒, 1us
 *        0xFFFF 为 uint16_t 的最大值, 是定时器2初始化的重装载值
 *        (1) 按键抖动导致统计不精确:测量的脉冲的时间一直是一个数字, 不知道什么原因 - 在 GPIO 中断(按键)中延时了, 导致统计出错
 * 
 */
void timer2_cap_test()
{
    uint32_t temp;

    if(TIM2_CH1_CAPTURE_STA & TIM2_CAPTURE_COMPLETE){
        temp = (uint32_t)(TIM2_CH1_CAPTURE_STA & 0x3F); // 超时次数
        printf("pulse number:%ld, curr timer cnt:%d\r\n", temp, TIM2_CH1_CAPTURE_VAL);
        temp *= 0xFFFF; // uint16_t_max = 65535
        temp += TIM2_CH1_CAPTURE_VAL;
        printf("pulse width:%d us = %d s\r\n", temp, temp / 1000000);
        TIM2_CH1_CAPTURE_STA = 0; // start next capture
        TIM2_CH1_CAPTURE_VAL = 0;
    }

    return;
}

void ADC_test()
{
    uint16_t adcVal = 0;

    Adc_Init();

    while(1){
        adcVal = Get_Adc_Average(ADC_VAL_CH1, ADC_GET_TEST_COUNT);
        printf("ADC val:%d\nvoltage:%02fV\n", adcVal, adcVal2Voltage(adcVal));
        delay_ms(3000);
    }
}

void dac_test()
{
    // Dac1_Init();

    // 让 DAC 输出三角波
    //^^^^^^^^^^^^^^^
    // 如何将它输出到屏幕上?
    // while(1){
        static uint8_t dacVal = 0;
        Dac1_Set_Vol(dacVal * DAC_OUTPUT_MAX_VOL);
        dacVal = !dacVal;
    // }
}


#if DMA_TEST_ENABLE
void dma_test()
{
    uint16_t t = 0, ix = 0;
    uint32_t sendNum;
    float progress = 0;	// 传输进度条
    uint8_t oledChar[30];

    // DMA 测试
    // UART1 TX - DMA1_Channel 4
    // 外设为串口1, 外设地址位置 USART1->DR 寄存器的地址
    // 存储器为 sendBuff, 存储器地址为数组首地址
    // 传输数据量:长 (TEXT_LENGTH+2) * 100 bytes
    // DMA_Config(DMA1_Channel4, (uint32_t)&USART1->DR, (uint32_t)sendBuff, (TEXT_LENGTH + 2) * SEND_COUNT);

    // 拷贝要发送的数据到数组中(存储器地址)
    for(ix = 0; ix < ((TEXT_LENGTH + 2) * SEND_COUNT); ix++){
        if(t >= TEXT_LENGTH){// 加入换行符
            sendBuff[ix++] = 0x0D;
            sendBuff[ix++] = 0x0A;
            t = 0;
        }
        else{
            sendBuff[ix] = TEXT_TO_SEND[t++];
        }
        // printf("%d %d\n", ix, t);
    }

    // 按键按下一次, 就传输一次数据
    // 开始一次 DMA 传输
    // 等待 DMA 传输完成, 此时我们可以做一些其他事情(不需要 CPU 参与)
    // 实际应用中, DMA 传输期间可以执行另外的任务

    printf("DMA start transmit, send size:%d\r\n", ((TEXT_LENGTH + 2) * SEND_COUNT));
    USART1->CR3 |= (1 << 7); // 使能串口的 DMA 传输

    DMA_Enable(DMA1_Channel4); // 开始传输一次

    while(1){
        if(DMA1->ISR & (1 << 13)){
            DMA1->IFCR |= 1 << 13;	// 清除 DMA1 ch4 的传输完成标志
            // printf("DMA transfer completed.\r\n");
            break;
        }
        // 获取当前传输数据的百分比(总共要传输 100 次)
        sendNum = DMA1_Channel4->CNDTR; // 得到当前还剩余多少个数据
        progress = 1 - (sendNum / ((TEXT_LENGTH + 2) * SEND_COUNT));
        progress *= 100; // 获得百分比
#if OLED_SCREEN_ENABLE
        // 串口在使用, 直接使用 printf() 打印内容可能会异常
        sprintf(oledChar, "send:%d%%", progress);
        OLED_ShowString(50, 50, oledChar);
#endif
        // printf("sendNum:%d\r\n", sendNum);

    }
    printf("DMA test finish\r\n");
}
#endif


void eeprom_test()
{
    #define EEPROM_WRITE_START_ADDR		0x00

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
    const uint8_t textSize = sizeof(TEXT); // 保存结尾 '\0'
    uint8_t buffer[256];

    while(AT24CXX_Check()){
        if(++count > 10){
            printf("detect EEPROM failed\n");
            return;
        }
        printf("Can't detect EEPROM:24C02, try again\n");
        delay_ms(500);
    }

    // EEPROM 掉电不丢失， 
    // 为了保证数据的可靠性, 通常会加上 CRC, 每次读取前校验一下 CRC
    delay_ms(1000);
    printf("write:%s\n", TEXT);
    AT24CXX_Write(EEPROM_WRITE_START_ADDR, (uint8_t*)TEXT, textSize);	

    delay_ms(1000);
    AT24CXX_Read(EEPROM_WRITE_START_ADDR, buffer, textSize);
    printf("read:%s\n", buffer);
#else
    while(AT24CXX_Check()){
        if(++count > 10){
            printf("detect EEPROM failed\n");
            return;
        }
        printf("Can't detect EEPROM:24C02, try again\n");
        delay_ms(500);
    }

    printf("strcut size:%d\r\n", sizeof(boot_mode_t));

    boot_mode.app_mode = 1;
    printf("boot mode before:%d %d %d\r\n", boot_mode.factory_mode, boot_mode.app_mode, boot_mode.ota_mode);
    AT24CXX_Write(EEPROM_WRITE_START_ADDR, (uint8_t*)&boot_mode, sizeof(boot_mode_t));

    memset(&boot_mode, 0, sizeof(boot_mode_t));
    printf("boot mode inner:%d %d %d\r\n", boot_mode.factory_mode, boot_mode.app_mode, boot_mode.ota_mode);

    AT24CXX_Read(EEPROM_WRITE_START_ADDR, (uint8_t*)&boot_mode, sizeof(boot_mode_t));
    printf("boot mode after:%d %d %d\r\n", boot_mode.factory_mode, boot_mode.app_mode, boot_mode.ota_mode);
#endif
    return;
}

void external_flash_test(uint8_t dir)
{
// 片外 flash 的地址也是从 0x0 开始编址的
#define FLASH_ADDR_START        0x00
#define FLASH_ADDR_END          8 * 1024 * 1024 - 1     // 8 Mbytes

    uint8_t count = 0;
    const uint8_t TEXT[] = "STM32_TEST_PROJECT";
    const uint8_t textSize = sizeof(TEXT); // 保存结尾 '\0'
    uint8_t buffer[256];
    const uint32_t FLASH_SIZE = 8 * 1024 * 1024;	// falsh 大小:8M


    // 读取 falsh ID
    while(SPI_Flash_ReadID() != W25Q64){
        if(++count > 10){
            printf("falash read error\n");
            return;
        }
        printf("can't read flash, try again\n");
        delay_ms(500);
    }

    /******************* flash 负载均衡, 不能一直往一个地址读写东西 *******************/
    // 写入 flash 并读取
    // SPI_Flash_Write((uint8_t*)TEXT, FLASH_SIZE - 100, textSize);
    if(dir == 0){
        SPI_Flash_Write((uint8_t*)TEXT, FLASH_ADDR_START, textSize);
        printf("write addr:0x%08X, data:%s\n", FLASH_ADDR_START, TEXT);
        delay_ms(1000);
    }
    else if(dir == 1){
        // SPI_Flash_Read(buffer, FLASH_SIZE - 100, textSize);
        SPI_Flash_Read(buffer, FLASH_ADDR_START, textSize);
        printf("read addr:0x%08X data:%s\n", FLASH_ADDR_START, buffer);
        delay_ms(1000);
    }
    else{

    }
    return;
}

void remote_test()
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


#define FLASH_SAVE_ADDR 	0x08020000 // flash 写入地址, 从第 128K 的地方开始写 
void inner_flash_test(uint8_t read_write_flag)
{
    const uint8_t TEXT[] = "This is a hello world test project.";
    const uint8_t textSize = sizeof(TEXT) + 1;	// '\0'
    uint8_t buffer[256];

    // flash 写入要注意:
    // (1) 必须为偶数
    // (2) 地址要大于本代码占用 flash 大小 + 0x08000000, 还要小于 flash 总大小, 不能把代码段修改了
    // 总大小:0x08000000 - 0x08000000 + 1024 * 256 = 08040000
    // Program Size: Code=53974 RO-data=8078 RW-data=420 ZI-data=3908  
    // 目前代码量:52K 左右
    if(read_write_flag){
        printf("write:%s\n", TEXT);
        // uint8_t arr[] -> uint16_t*
        STMFLASH_Write(FLASH_SAVE_ADDR, (uint16_t*)TEXT, textSize);
        delay_ms(1000);
    }
    else{
        STMFLASH_Read(FLASH_SAVE_ADDR, (uint16_t*)buffer, textSize);
        printf("read:%s\n", buffer);
        delay_ms(1000);
    }
    return;
}

void tp_test()
{

}



/**
 * @brief 显示鼠标的坐标值
 * 
 * @param x 在 LCD 上显示的坐标位置
 * @param y 
 * @param pos 坐标值
 */
void Mouse_Show_Pos(u16 x, u16 y, short pos)
{
    if(pos<0)
    { 
#if LCD_SCREEN_ENABLE
        LCD_ShowChar(x, y, '-', 16, 0); //显示负号
#endif
        pos = -pos; //转为正数
    }
    else 
#if LCD_SCREEN_ENABLE
        LCD_ShowChar(x, y, ' ', 16, 0);//去掉负号
#endif

#if LCD_SCREEN_ENABLE
    LCD_ShowNum(x + 8, y, pos, 5, 16); //显示值 
#endif
    return;
}

/**
 * @brief while(1) 中调用
 * 
 */
void mouse_test()
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

    if(flag){
        uint32_t *p = (uint32_t*)mymalloc(sizeof(uint32_t) * 500);
        if(!p){
            printf("malloc failed\r\n");
            return;
        }
        mymemset(p, 0, sizeof(uint32_t) * 500);
        // save pointer
        pArr[pointerNum++] = p;
        printf("memory alloc, address:0x%08X\r\n", p);
        printf("memory used rate:%d\r\n", mem_perused());
    }
    else{
        uint32_t *p;
        if(pointerNum <= 0){
            printf("no memory to free\r\n");
            return;
        }
        p = pArr[--pointerNum];
        if(!p){
            printf("no memory to free\r\n");
            return;
        }
        myfree(p);
        printf("memory free, address:0x%08X\r\n", p);
        printf("memory used rate:%d\r\n", mem_perused());
    }
    return;
}

/**
 * @brief 读取 SD card 指定扇区的内容
 * 
 * @param sect 第几扇区
 * @param dir 0:read; 1-write
 */
void sdcard_read_write_sectorx_test(u32 sect, u8 dir)
{
    u8 *buf;
    u16 i;

    buf = (u8*)mymalloc(512 * sizeof(u8));
    mymemset(buf, 0, 12);
    if(!dir){
        if(SD_ReadDisk(buf, sect, 1) == MSD_RESPONSE_NO_ERROR){
            printf("Sector %d data:\r\n", sect);
            for(i = 0; i < 512; ++i){
                printf("%02X ", buf[i]);
            }
            printf("\r\n");
        }
        else{
            printf("sdcard read error\r\n");
        }
    }
    else{
        strcpy(buf, "123456789");
        if(SD_WriteDisk(buf, sect, 1) == MSD_RESPONSE_NO_ERROR){
            printf("sector write success\r\n");
        }
        else{
            printf("sector write failed\r\n");
        }
    }
    myfree(buf);
    return;
}

/**
 * @brief FATFS 提供的接口测试
 *        这文件系统真坑爹啊, 还是要多看看官方例程具体是怎么操作的!!!!!!!!!!!!!!!!
 * @param op 
 */
void fatfs_test(uint8_t op)
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
            // 仅仅打开当前目录, 不糊递归访问子目录
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
            // 难道 FATFS 不可以以读写方式打开文件 ??????????????
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
            // 0: 表示盘符
            // 格式化
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

/**
 * @brief FS API 测试
 * 
 * @param op 
 */
void fs_api_test(uint8_t op)
{
    switch(op){
        case 0:
        {
            char *fileName = "0:test1.txt";
            FRESULT res;
            int fd;
            char *buff = mymalloc(sizeof(uint8_t) * 256);
            if(!buff){
                printf("malloc error\r\n");
                return;
            }
            // 暂时还不知道怎么支持同时读写文件
            fd = open(fileName, FA_CREATE_NEW | FA_WRITE);
            if(fd < 0){
                printf("open ERROR, fd:%d\r\n", fd);
                free(buff);
                return;
            }
            res = write(fd, fileName, strlen(fileName) + 1);
            if(res < 0){
                printf("write ERROR, res:%d\r\n", res);
                free(buff);
                return;
            }
            printf("write len:%d\r\n", res);
                res = close(fd);
            if(res < 0) {
                printf("write ERROR, res:%d\r\n", res);
                free(buff);
                return;
            }

            // 再次打开文件 读文件
            fd = open(fileName, FA_READ);
            if(fd < 0){
                printf("open ERROR, fd:%d\r\n", fd);
                free(buff);
                return;
            }
            res = read(fd, buff, 156);
            if(res < 0){
                printf("write ERROR, res:%d\r\n", res);
                free(buff);
                return;
            }
            printf("read len:%d\r\n", res);
            res = close(fd);
            if(res < 0) {
                printf("write ERROR, res:%d\r\n", res);
                free(buff);
                return;
            }
            free(buff);
        }
        break;
        case 8:
            fs_test();
        break;
        case 9:
            fs_speed_test(1);
        default:
        break;
    }

    return;
}

