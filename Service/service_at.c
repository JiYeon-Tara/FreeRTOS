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


// 流程大约是:串口接收数据 -> 解析 -> 执行提前注册好的回调函数


// macro
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

// function declaration
static bool deal_at_cmd(const char *cmd, const char *para1, const char *para2);
static bool at_trans_start(const char *para1, const char *para2);
static bool at_LED_test(const char *para1, const char *para2);

// variables
const static at_callback_t atCmdList[] = {
    {"AT^START",                            at_trans_start},
    {"AT^LED",                              at_LED_test},
};
static uint8_t _g_at_send_temp_buff[512];
static AT_DIR_E atDir = AT_DIR_UART;


// function
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
    printf("token:%s\r\n", token);
    while(token != NULL && delimNum <= 2){ // 最多两个参数
        snprintf((char*)tempStr[delimNum], 64, "%s", token);
        ++delimNum;
        token = strtok(NULL, delim);
        printf("token:%s\r\n", token);
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


