/**
 * @file service_at.c
 * @author your name (you@domain.com)
 * @brief 通过串口实现 AT 命令
 * @version 0.1
 * @date 2022-11-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "service_at.h"
#include "usart.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include "led.h"
#include "oled.h"
#include "rtc.h"
#include "tp_test.h"
#include "stm_flash.h"
#include "test.h"
#include "bsp_config.h"

// 流程大约是:串口接收数据 -> 解析 -> 执行提前注册好的回调函数


/***********************
 *  MACRO
 ***********************/
#define _WHILE_LIST_NUM         50
#define AT_TRANS(...)             (service_at_send(__VA_ARGS__))

// typedef
typedef bool (*atFunc)(const char *para1, const char *para2);

typedef struct _at_callback_t{
    char    *atCmd;
    atFunc  atFun;
} at_callback_t;

typedef enum {
    AT_DIR_DEFAULT,
    AT_DIR_UART = AT_DIR_DEFAULT,
    AT_DIR_BLE,
    AT_DIR_BT,
    AT_DIR_NUM
} AT_DIR_E;


/***********************
 *  FUNCTION DECLARE
 ***********************/
static bool deal_at_cmd(const char *cmd, const char *para1, const char *para2);
static bool at_trans_start(const char *para1, const char *para2);
static bool at_trans_end(const char *para1, const char *para2);
static bool at_LED_test(const char *para1, const char *para2);
static bool at_oled_gpio_test(const char *para1, const char *para2);
static bool at_oled_display_test(const char *para1, const char *para2);
static bool at_rtc_set_test(const char *para1, const char *para2);
static bool at_dma_uart_test(const char *para1, const char *para2);
static bool at_eeprom_test(const char *para1, const char *para2);
static bool at_external_flash_test(const char *para1, const char *para2);
static bool at_tp_calibration(const char *para1, const char *para2);
static bool at_inner_flash_test(const char *para1, const char *para2);
static bool at_memory_manage_test(const char *para1, const char *para2);
static bool at_sdcard_test(const char *para1, const char *para2);
static bool at_fatfs_test(const char *para1, const char *para2);

// variables
const static at_callback_t atCmdList[] = {
    {"AT^START",                            at_trans_start},
    {"AT^END",                              at_trans_end},
    {"AT^LED",                              at_LED_test},
    {"AT^OLED_GPIO",                        at_oled_gpio_test},
    {"AT^OLED_DISPLAY",                     at_oled_display_test},
    {"AT^RTC_SET",                          at_rtc_set_test},
    {"AT^DMA_TEST",                         at_dma_uart_test},
    {"AT^EEPROM_TEST",                      at_eeprom_test},
    {"AT^EXT_FLASH_TEST",                   at_external_flash_test},
    {"AT^TP_CALIBRATE",                     at_tp_calibration}, 
    {"AT^INNER_FLASH_TEST",                 at_inner_flash_test},
    {"AT^MEMORY_TEST",                      at_memory_manage_test},
    {"AT^SDCARD_TEST",                      at_sdcard_test},
    {"AT^FATFS_TEST",                       at_fatfs_test}, 
};
static uint8_t _g_at_send_temp_buff[512];
static AT_DIR_E atDir = AT_DIR_UART;


/***********************
 * FUNCTION
 ***********************/
/**
 * @brief AT 命令字符串解析
 *        输入格式为:AT^START=1=2
 * 
 * @param input 
 * @return true/false
 */
bool service_at_parse(char *inputStr)
{
    const char delim[2] = "=";
    char tempStr[3][64];
    char *token;
    uint8_t delimNum = 0;

    memset(tempStr, 0, sizeof(tempStr));

    if(strlen(inputStr) == 0){
        return false;
    }

    // strtok() 不是一个可重入的函数, 内部会保存传入的字符串
    token = strtok(inputStr, delim); // 返回值指向子串
    // printf("token:%s\r\n", token);
    while(token != NULL && delimNum <= 2){ // 最多两个参数
        snprintf((char*)tempStr[delimNum], 64, "%s", token);
        ++delimNum;
        token = strtok(NULL, delim);
        // printf("token:%s\r\n", token);
    }

    return deal_at_cmd((const char *)tempStr[0], (const char *)tempStr[1], (const char *)tempStr[2]);
}

/**
 * @brief 通过 AT 命令执行相应的回调函数
 * 
 * @param cmd 
 * @param para1 
 * @param para2 
 * @return true/false 
 */
static bool deal_at_cmd(const char *cmd, const char *para1, const char *para2)
{
    int cmdNum = (sizeof(atCmdList) / sizeof(atCmdList[0]));
    int idx;

    for(idx = 0; idx < cmdNum; ++idx){
        if(!strcmp(cmd, atCmdList[idx].atCmd)){
            // 从 flash 读取标志位:判断是否从调试模式进入
            // service_factory_read_mmi(&nv_factory_tool);
            // if(nv_factory_tool.fromFactory == 1){

            // }
            // else{

            // }
            return atCmdList[idx].atFun(para1, para2);
        }
    }
}

/**
 * @brief AT 命令发送
 *        可以设置返回方向, 通过 BT/BLE/UART 等等发送
 * 
 * @param fmt 
 * @param ... 
 */
void service_at_send(const char *fmt, ...)
{
    va_list args;
    static bool mutex = false;
    int len = 0;

    if(mutex){
        printf("mutex:%d\r\n", mutex);
        return;
    }
    mutex = true;
    va_start(args, fmt); // args point to the first variable parameter
    // vsnprintf() 是个什么操作
    len = vsnprintf(_g_at_send_temp_buff, sizeof(_g_at_send_temp_buff), fmt, args);
    va_end(args);

    if(len > 0){
        switch(atDir){
            case AT_DIR_UART:
                printf("%s", _g_at_send_temp_buff);
                break;
            case AT_DIR_BLE:
                // service_ble_send(_g_at_send_temp_buff, sizeof(_g_at_send_temp_buff));
                break;
            case AT_DIR_BT:
                // service_ble_send(_g_at_send_temp_buff, sizeof(_g_at_send_temp_buff));
                break;
            default:
                printf("unknown dir:%d\r\n", atDir);
                break;
        }
    }

    mutex = false;

    return;
}

static bool at_trans_start(const char *para1, const char *para2)
{
    AT_TRANS("OK\r\n");
}

static bool at_trans_end(const char *para1, const char *para2)
{
    AT_TRANS("OK\r\n");
}


static bool at_LED_test(const char *para1, const char *para2)
{
    LED_PIN_E ledNum;
    LED_STATUS_E ledState;
    if(!para1 || !para2){
        AT_TRANS("ERROR\r\n");
    }

    ledNum = (LED_PIN_E)atoi(para1);
    ledState = atoi(para2);

    // if(ledNum == )
    AT_TRANS("%d:%d\r\nOK\r\n", ledNum, ledState);

    if(!led_state_change(ledNum, ledState)){
        AT_TRANS("ERROR\r\n");
        return false;
    }

    return true;
}

static bool at_double_div_test(const char *para1, const char *para2)
{
    // char arr[3][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}};
    // int row, col;

    // for(row = 0; row < 3; ++row){
    //     for(col = 0; col < 4; ++col){
    //         printf("row:%d col:%d elem:%d\r\n", row, col, arr[row][col]);
    //     }
    // }

    // 二维数组的排列顺序
    // arr[rowNum][colNum]
    // arr[0][0] arr[0][1] arr[0][2] arr[0][3]
    // arr[1][0] arr[1][1] arr[1][2] arr[1][3]
}

static bool at_oled_gpio_test(const char *para1, const char *para2)
{
    uint8_t levelFlag;
    if(!para1){
        AT_TRANS("ERROR");
        return false;
    }

    levelFlag = (uint8_t)atoi(para1);

    AT_TRANS("level:%d\r\n", levelFlag);
    if(levelFlag == 0){
        // OLED_CS = 0;
        // OLED_RST =0;
        // OLED_DC = 0;
        // OLED_SCLK = 0;
        // OLED_SDIN = 0;
    }
    else{
        // OLED_CS = 1;
        // OLED_RST = 1;
        // OLED_DC = 1;
        // OLED_SCLK = 1;
        // OLED_SDIN = 1;
 
    }
    AT_TRANS("OK\r\n");
    return true;
}

static bool at_oled_display_test(const char *para1, const char *para2)
{
    uint8_t displayNum;
    if(!para1){
        AT_TRANS("ERROR");
        return false;
    }

    displayNum = (uint8_t)atoi(para1);

    AT_TRANS("displayNum:%d\r\n", displayNum);
#if OLED_SCREEN_ENABLE
    switch(displayNum){
        case 1:

            OLED_ShowString(0, 0, "static bool at_oled_display_test(const char *para1, const char *para2)");
            break;
        case 2:
            OLED_ShowChar(0, 50, '.');
            break;
        case 3:
            OLED_ShowString(0, 0, "abcdefghigklmnopqrstuvwsyzabcdefghigklmnopqrstuvwsyz");
            break;
        case 4:
            OLED_Clear();
            OLED_DrawBMP(0, 0, 127, 7, BMP1);
        break;
        case 5:
            OLED_Clear();
            OLED_DrawBMP(0, 0, 127, 7, BMP2);
        break;
        default:
            break;
    }
#endif
    AT_TRANS("OK\r\n");
    return true;
}

static bool at_rtc_set_test(const char *para1, const char *para2)
{
    enum{
        YEAR = 0,
        MONTH,
        DAY,
        HOUR,
        MINUTE,
        SECOND
    };
    uint8_t idx;
    uint16_t val;
    if(!para1 || !para2){
        AT_TRANS("ERROR");
        return false;
    }

    idx = (uint8_t)atoi(para1);
    val = (uint16_t)atoi(para2);
    printf("idx:%d, val:%d\r\n", idx, val);

    switch(idx){
        case YEAR:
            if(val >= 0){
                RTC_Get();
                RTC_Set(val, calendar.w_month, calendar.w_date, calendar.hour, calendar.min, calendar.sec);
            }
            break;
        case MONTH:
            if(val >= 0 && val <= 12 ){
                RTC_Get();
                RTC_Set(calendar.w_year, val, calendar.w_date, calendar.hour, calendar.min, calendar.sec);
            }
            break;
        case DAY:
            if(val >= 0 && val <= 31){
                RTC_Get();
                RTC_Set(calendar.w_year, calendar.w_month, val, calendar.hour, calendar.min, calendar.sec);
            }
            break;
        case HOUR:
            if(val >= 0 && val <= 24){
                RTC_Get();
                RTC_Set(calendar.w_year, calendar.w_month, calendar.w_date, val, calendar.min, calendar.sec);
            }
            break;
        case MINUTE:
            if(val >= 0 && val <= 60){
                RTC_Get();
                RTC_Set(calendar.w_year, calendar.w_month, calendar.w_date, calendar.hour, val, calendar.sec);
            }
            break;
        case SECOND:
            if(val >= 0 && val <= 60){
                RTC_Get();
                RTC_Set(calendar.w_year, calendar.w_month, calendar.w_date, calendar.hour, calendar.min, val);
            }
            break;
        default:
            break;
    }
    AT_TRANS("OK\r\n");
    return true;
}

static bool at_dma_uart_test(const char *para1, const char *para2)
{
    enum{
        DIR_PERIPHERAL_TO_RAM,
        DIR_RAM_TO_PERIPHERAL
    };
    uint8_t dir;
    if(!para1){
        AT_TRANS("ERROR\r\n");
        return false;
    }
    
    dir = (uint8_t)atoi(para1);

    if(dir == DIR_PERIPHERAL_TO_RAM){

    }
    else if(dir == DIR_RAM_TO_PERIPHERAL){
#if DMA_TEST_ENABLE
        dma_test();
#endif
    }
    else{

    }

    AT_TRANS("OK\r\n");
    return true;
}

static bool at_eeprom_test(const char *para1, const char *para2)
{
    eeprom_test();
    AT_TRANS("OK\r\n");
    return true;
}

static bool at_external_flash_test(const char *para1, const char *para2)
{
    uint8_t dir = 0;
    if(!para1){
        AT_TRANS("ERROR\r\n");
        return false;
    }
    dir = (uint8_t)atoi(para1);
    external_flash_test(dir);
    AT_TRANS("OK\r\n");
    return true;
}

static bool at_tp_calibration(const char *para1, const char *para2)
{
#if TP_ENABLE
    enter_tp_adjust();
#endif // TP_ENABLE
    AT_TRANS("OK\r\n");
    return true;
}

static bool at_inner_flash_test(const char *para1, const char *para2)
{
    uint8_t dir = 0;
    if(!para1){
        AT_TRANS("ERROR\r\n");
        return false;
    }
    dir = (uint8_t)atoi(para1);
    printf("dir:%d\r\n");
    inner_flash_test(dir);
    AT_TRANS("OK\r\n");
    return true;
}

static bool at_memory_manage_test(const char *para1, const char *para2)
{
    uint8_t dir = 0;
    if(!para1){
        AT_TRANS("ERROR\r\n");
        return false;
    }
    dir = (uint8_t)atoi(para1);
    printf("dir:%d\r\n");
    memmang_test(dir);
    AT_TRANS("OK\r\n");
    return true;
}

static bool at_sdcard_test(const char *para1, const char *para2)
{
    uint8_t dir = 0;
    uint8_t sect = 0;
    if(!para1 || !para2){
        AT_TRANS("ERROR\r\n");
        return false;
    }
    dir = (uint8_t)atoi(para1);
    sect = (uint8_t)atoi(para2);
    printf("dir:%d sect:%d\r\n", dir, sect);
    sdcard_read_write_sectorx_test(sect, dir);
    AT_TRANS("OK\r\n");
    return true; 
}

static bool at_fatfs_test(const char *para1, const char *para2)
{
    enum {
        FATFS_TEST,
        FS_API_TEST,
        TEST_MAX
    };
    uint8_t op = 0;
    uint8_t testAPI = 0;
    if(!para1 || !para2){
        AT_TRANS("ERROR\r\n");
        return false;
    }
    testAPI = (uint8_t)atoi(para1);
    op = (uint8_t)atoi(para2);
    if(testAPI == 0)
        fatfs_test(op);
    else
        fs_api_test(op);
    AT_TRANS("OK\r\n");
    return true; 
}
