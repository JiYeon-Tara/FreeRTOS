#ifndef __COMMAND_PARSE_H__
#define __COMMAND_PARSE_H__
#include <stdint.h>

/**************************** global varible ******************************/

/**************************** macro definition ******************************/
//用于串口控制 LED 的命令
#define LED0_ON     1
#define LED0_OFF    2
#define LED1_ON     3
#define LED1_OFF    4
#define COMMAND_ERR 0xFF

/**************************** function declaration ******************************/

// uint8_t command_parse(uint8_t *command);

#endif //end __COMMAND_PARSE_H__

