#ifndef __KEY_H
#define __KEY_H	 


#include "util.h"



//通过循环扫描的方式实现按键
#define KEY0_IDX    0 // KEY0
#define KEY1_IDX    1 // KEY1
#define KEY_WKUP_IDX    2 // KEY_WKUP

#define KEY0_PRES	0		//KEY0按下
#define KEY1_PRES	1		//KEY1按下
#define WKUP_PRES	2		//WK_UP按下
#define NO_KEY_PRES -1      // 无按键按下

#define KEY0  PCin(5)   	//PC5
#define KEY1  PAin(15)	 	//PA15 
#define WK_UP PAin(0)	 	//PA0  WK_UP
/**************************************************************************
 * MACRO
 **************************************************************************/
//PC5:KEY0
#define KEY0_GPIO                  GPIO_C
#define KEY0_PORT                  5
#if NVIC_TEST == 1
#define KEY0_PRIEMPTION_PRIORITY   NVIC_PREEMPTION_PRIORITY_HIGH
#else
#define KEY0_PRIEMPTION_PRIORITY   NVIC_PREEMPTION_PRIORITY_MIDDLE
#endif
#define KEY0_SUB_PRIORITY          NVIC_SUBPRIORITY_MIDDLE
#define KEY0_NVIC_GROUP            2 // obsolete

//PA15:KEY1
#define KEY1_GPIO                  GPIO_A
#define KEY1_PORT                  15
#if NVIC_TEST == 1
#define KEY1_PRIEMPTION_PRIORITY   NVIC_PREEMPTION_PRIORITY_LOW
#else
#define KEY1_PRIEMPTION_PRIORITY   NVIC_PREEMPTION_PRIORITY_MIDDLE
#endif
#define KEY1_SUB_PRIORITY          NVIC_SUBPRIORITY_MIDDLE
#define KEY1_NVIC_GROUP            2 // obsolete

//PA0:WK_UP
#define WKUP_GPIO                   GPIO_A
#define WKUP_PORT                   0
#define WKUP_PRIEMPTION_PRIORITY    NVIC_PREEMPTION_PRIORITY_MIDDLE
#define WKUP_SUB_PRIORITY           NVIC_SUBPRIORITY_MIDDLE
#define WKUP_NVIC_GROUP             2 // obsolete

// 按键模式
typedef enum{
    SINGLE_SHOT_MODE = 0,    // 不支持连续按
    CONTINUE_MODE            // 支持连按
} KEY_MODE_E;
     
void KEY_Init(void);		//IO初始化
int8_t KEY_Scan(KEY_MODE_E mode);
void set_key_pressed(int key_idx, bool pressed);
int key_pressed(void);
void sm_key_reset(void);
void key_reset(void);

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