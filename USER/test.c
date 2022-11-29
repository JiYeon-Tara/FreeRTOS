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
#include "board_config.h"
#include "bsp_config.h"
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h" 
#include "key_stm.h" 
#include "service_usart.h"
#include "watch_dog_timer.h"
#include "timer.h"
#include "LCD.h"
#include "ssd_1306.h"
#include "usmart.h"
#include "exti.h"
#include "third_party_config.h"
#include "at_cmd_parse.h"
#include "string.h"
#include "service_at.h"

 
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


void bsp_init()
{
    uint8_t lcd_id[20];
    uint8_t temp[50];
    uint8_t time[50];
    uint8_t cpuTemp = 50;
    uint16_t year = 2022, month = 9, day = 1, hour = 8, min = 0, sec = 0; 
    uint8_t count = 0;

    Stm32_Clock_Init(9); 		 //系统时钟设置
    delay_init(72000000);	     //延时初始化
    delay_ms(100);
#if USART1_ENABLE
    // 初始化之后才可以使用 printf()
    uart1_init(72000000, 115200);	 //串口初始化为9600
#endif

    LCD_Init();
    RTC_Init();

#if LED_ENABLE
    LED_Init();
#endif

#if KEY_ENABLE
    KEY_Init();
#endif

#if ADC_ENABLE
    Adc_Init();
#endif

#if DAC_ENABLE
    Dac1_Init();
#endif

#if EEPROM_ENABLE
    AT24CXX_Init();
#endif

#if FLASH_ENABLE
    SPI_Flash_Init();
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

    sprintf((char*)lcd_id, "LCD ID:%04X", lcddev.id);
    sprintf((char*)temp, "CPU temperature:%d", cpuTemp);
    sprintf((char*)time, "Time:%d-%d-%d %d:%d:%d  Week:%d", calendar.w_year, calendar.w_month,calendar.w_date, \
                                                   calendar.hour, calendar.min, calendar.sec, calendar.week);
    
    LCD_ShowString(0, 0, 240, 16, 16, lcd_id);
    LCD_ShowString(0, 16, 240, 16, 16, temp);
    LCD_ShowString(0, 32, 240, 16, 16, time);
}


void service_init()
{
#if MALLOC_TEST_ENABLE
    mem_init();
#endif
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
#else if INT_KEY_ENABLE
    EXTI_Init();
#endif
}

void usart_test()
{
#if USART1_ENABLE
    uint8_t len;
    uint8_t ix = 0;

    uart_init(72000000, 9600);	 //串口初始化为9600
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
#elseif WWATCH_DOG_TEST_ENABLE
    
#endif
}

void oled_screen_test()
{

}

void lcd_screen_test()
{
    uint8_t lcd_id[20];

    LCD_Init();
    printf("LCD_Init\n");
    sprintf((char*)lcd_id, "LCD ID:%04X", lcddev.id);

    LCD_ShowString(0, 0, 240, 16, 16, lcd_id);
    LCD_Draw_Circle(20, 20, 10);
    LCD_DrawLine(40, 40, 80, 80);	
}

void rtc_test()
{
    setRealTimeRtc();
    RTC_Init();
    RTC_Set(calendar.w_year, calendar.w_month, calendar.w_date, calendar.hour, calendar.min, calendar.sec);
    while(1){
        printf("%d-%d-%d %d:%d:%d, week:%d\n", calendar.w_year, calendar.w_month, calendar.w_date, \
                                               calendar.hour, calendar.min, calendar.sec, calendar.week);
        delay_ms(5000);
    }
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
            USART_RX_STA = 0;

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
        }
    // }
    return;
}

void dlps_test()
{
    WKUP_Init();
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

// #if DMA_TEST_ENABLE

// #endif

void dma_test()
{
    uint16_t t = 0, ix = 0;
    uint32_t sendNum;
    float progress = 0;	// 传输进度条
    // DMA 测试
    uint8_t sendBuff[5168];
    const uint8_t TEXT_TO_SEND[] = "hello, world";
    #define TEXT_LENGTH 	(sizeof(TEXT_TO_SEND) - 1)	// 要发送的数据长度
    #define SEND_COUNT		20

    printf("dma test init\r\n");

    // UART1 TX - DMA1_Channel4
    // 外设为串口1, 外设地址位置 USART1->DR 寄存器的地址
    // 存储器为 sendBuff, 存储器地址为数组首地址
    // 传输数据量:长 (TEXT_LENGTH+2) * 100 bytes
    DMA_Config(DMA1_Channel4, (uint32_t)&USART1->DR, (uint32_t)sendBuff, (TEXT_LENGTH + 2) * SEND_COUNT);

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
    while(1){

        while(1){
            // 等待通道 4 传输完成
            if(DMA1->ISR & (1 << 13)){
                DMA1->IFCR |= 1 << 13;	// 清除 DMA1 ch4 的传输完成标志
                // printf("DMA transfer completed.\r\n");
                break;
            }
            
            // 获取当前传输数据的百分比(总共要传输 100 次)
            sendNum = DMA1_Channel4->CNDTR; // 得到当前还剩余多少个数据
            progress = 1 - (sendNum / ((TEXT_LENGTH + 2) * SEND_COUNT));
            progress *= 100; // 获得百分比
            delay_ms(3000);
            // printf("sendNum:%d, progress:%f\n", sendNum, progress);
        }
        // printf("loop");
    }
    printf("DMA test finish\r\n");
}


#define EEPROM_WRITE_START_ADDR		0x00
void eeprom_test()
{
    uint8_t count = 0;
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
    // printf("write:%s\n", TEXT);
    // AT24CXX_Write(EEPROM_WRITE_START_ADDR, (uint8_t*)TEXT, textSize);	
    delay_ms(1000);
    AT24CXX_Read(EEPROM_WRITE_START_ADDR, buffer, textSize);
    printf("read:%s\n", buffer);

    return;
}


void flash_test()
{
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
    // printf("write:%s\n", TEXT);
    delay_ms(1000);
    SPI_Flash_Read(buffer, FLASH_SIZE - 100, textSize);
    printf("read:%s\n", buffer);

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


#define FLASH_SAVE_ADDR 	0x08020000 // flash 写入地址, 
void inner_flash_test()
{
    const uint8_t TEXT[] = "This is a hello world test project.";
    const uint8_t textSize = sizeof(TEXT) + 1;	// '\0'
    uint8_t buffer[256];

    // (1) 必须为偶数
    // (2) 地址要大于本代码占用 flash 大小 + 0x08000000, 还要小于 flash 总大小
    // 总大小:0x08000000 - 0x08000000 + 1024 * 256 = 08040000
    // Program Size: Code=33682 RO-data=8078 RW-data=420 ZI-data=3908  
    printf("write:%s\n", TEXT);
    // uint8_t arr[] -> uint16_t*
    STMFLASH_Write(FLASH_SAVE_ADDR, (uint16_t*)TEXT, textSize);
    delay_ms(1000);
    STMFLASH_Read(FLASH_SAVE_ADDR, (uint16_t*)buffer, textSize);
    printf("read:%s\n", buffer);
    delay_ms(1000);
}

#if MALLOC_TEST_ENABLE
void memmang_test()
{
    uint32_t *pArr = (uint32_t*)mymalloc(sizeof(uint32_t) * 500);
    printf("p:%d, memory usage:%d\n", (uint32_t)pArr, mem_perused());
    myfree(pArr);
    printf("p:%d, memory usage:%d\n", (uint32_t)pArr, mem_perused());
}
#endif




