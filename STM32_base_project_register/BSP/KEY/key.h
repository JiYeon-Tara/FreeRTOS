#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

//通过循环扫描的方式实现按键
#define KEY0_PRES	1		//KEY0按下
#define KEY1_PRES	2		//KEY1按下
#define WKUP_PRES	3		//WK_UP按下

#define KEY0  PCin(5)   	//PC5
#define KEY1  PAin(15)	 	//PA15 
#define WK_UP PAin(0)	 	//PA0  WK_UP
	 
void KEY_Init(void);		//IO初始化
u8 KEY_Scan(u8 mode);		//按键扫描函数

//通过位带操作实现按键输入
// #define KEY0    PCin(5)     //PC5
// #define KEY1    PAin(15)    //PA15
// #define WK_UP   PAin(0)     //PA0 WK_UP

// #define KEY0    (1 << 5)    //KEY0 PC5
// #define KEY1    (1 << 15)   //KEY1 PA15
// #define WK_UP   (1 << 0)    //WK_UP PA0

// #define KEY0_GET()  ((GPIOC->IDR & (KEY0)) ? 1 : 0)
// #define KEY1_GET()  ((GPIOA->IDR & (KEY1)) ? 1 : 0)
// #define WKUP_GET()  ((GPIOA->IDR & (WK_UP)) ? 1 : 0)

#endif
