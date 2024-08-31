#include "key_stm.h"
#include "delay.h"
#include "bsp_config.h"
#include "board_config.h"
#include "exti.h"
#if DLPS_ENABLE
#include "dlps.h"
#endif

#include "ulog.h"

// WK_UP, PA0
// KEY0, PC5
// KEY1, PA15, 和 JTAG 共用的 GPIO, 所以要禁用 JTAG, 使能 SWD
/******************* WK_UP 按键和红外共用一个 GPIO, 需要拔掉跳线帽 *******************/


static bool key0_pressed = false;
static bool key1_pressed = false;
static bool key_wkup_pressed = false;

/**
 * @brief 按键初始化函数-
 * 		  PA0, PA15 和 PC5 设置成输入 
 * 
 */
void KEY_Init(void)
{
    RCC->APB2ENR |= 1 << 2; // 使能PORTA时钟
    RCC->APB2ENR |= 1 << 4; // 使能PORTC时钟
    JTAG_Set(SWD_ENABLE); // 关闭JTAG, 开启SWD, JTAG 使用了 PA15

#if LOOP_KEY_ENABLE == 1
    GPIOA->CRH &= 0X0FFFFFFF; // PA15设置成输入	  
    GPIOA->CRH |= 0X80000000; // 上拉 / 下拉输入模式		 
    GPIOA->ODR |= (1 << 15); // PA15上拉, PA0(WK_UP)默认下拉

    GPIOC->CRL &= 0XFF0FFFFF; // PC5设置成输入	  
    GPIOC->CRL |= 0X00800000; // 上拉 / 下拉输入模式
    GPIOC->ODR |= 1 << 5; // PC5上拉, 通过输出数据寄存器设置上拉, 默认下拉
#endif

#if DLPS_ENABLE == 0 // WK_UP 作为普通的按键, 而不是低功耗唤醒按键时
#if TEMP_ENABLE == 0 // DS 18B20 会用到这个 GPIO
    GPIOA->CRL &= 0XFFFFFFF0; // PA0 设置成输入, 模拟输入模式	  
    GPIOA->CRL |= 0X00000008; // 上拉 / 下拉输入模式
    GPIOA->ODR &= 0xFFFFFFFF; // 设置下拉， GPIOA->ODR &= 0xFFFFFFF7
#endif /* TEMP_ENABLE */
#endif // DLPS_ENABLE
    
    // 中断按键测试
#if INT_KEY_ENABLE
    EXTI_Init();
#endif

    LOG_I("key init\r\n");
    return;
} 

/**
 * @brief 按键处理函数, 轮询方式
 * 		  返回按键值
 *        使用了 static 类型的变量, 不是一个可重入的函数, 使用多线程的时候要注意这里!!!
 * 
 * @param mode mode:0,不支持连续按;1,支持连续按;
 * @return u8 返回值：0，没有任何按键按下; KEY0_PRES，KEY0按下; KEY1_PRES，KEY1按下; WKUP_PRES，WK_UP按下 
 * 		   注意此函数有响应优先级,KEY0>KEY1>WK_UP!!
 */
int8_t KEY_Scan(KEY_MODE_E mode)
{
    static u8 key_up = 1;//按键按松开标志
    if(mode == CONTINUE_MODE)
        key_up = 1;  //支持连按	
    
    // KEY_UP按键 - 要断开 PA0 和 BS18B20 的跳线帽
    if(key_up && (KEY0==0 || KEY1==0 || WK_UP == 1)) 
    {
        delay_ms(10);//去抖动 
        key_up = 0;
        if(KEY0 == 0)
            return KEY0_PRES;
        else if(KEY1 == 0)
            return KEY1_PRES;
        else if(WK_UP == 1)
            return WKUP_PRES; 
    }
    else if(KEY0==1 && KEY1==1 && WK_UP == 0)
        key_up = 1;

    return NO_KEY_PRES;// 无按键按下
}

/**
 * @brief 中断按键
 *        配置 GPIO 时钟 -> 配置为输入 -> 配置中断 -> 打开中断
 * 		  写中断服务函数
 */


void set_key_pressed(int key_idx, bool pressed)
{
    switch (key_idx) {
        case KEY0_IDX:
            key0_pressed = pressed;
            break;
        case KEY1_IDX:
            key1_pressed = pressed;
            break;
        case KEY_WKUP_IDX:
            key_wkup_pressed = pressed;
            break;
        default:
            break;
    }
}


/**
 * @brief 获取当前被按下的键
 * 
 * @return int 
 */
int key_pressed(void)
{
    if (key0_pressed) {
        return KEY0_PRES;
    } else if (key1_pressed) {
        return KEY1_PRES;
    } else if (key_wkup_pressed) {
        return WKUP_PRES;
    } else {
        return NO_KEY_PRES;
    }
}

// 恢复所有按键状态, main loop 最后调用
void key_reset(void)
{
    set_key_pressed(KEY0_IDX, false);
    set_key_pressed(KEY1_IDX, false);
    set_key_pressed(KEY_WKUP_IDX, false);
}

