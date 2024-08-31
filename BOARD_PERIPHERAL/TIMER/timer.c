#include "timer.h"
#include "led.h"
#include "sys.h"

#include "util.h"
#include "ulog.h"

// 1.硬件定时器数量有限, 一般用于什么场合? 还分高级定时器(支持的功能更多,数量少)和普通定时器
//2. 软件定时器如何实现(链表)?


/*********************************************************************************
 * MACRO
 *********************************************************************************/
#define TIMER1_CH1_PWM_VAL          TIM1->CCR1         // timer1 PWM val, 调节占空比

// #define TIM2_CAPTURE_COMPLETE        (1 << 7)
// #define TIM2_CAP_HIGH_LEVEL          (1 << 6)
// #define TIM2_OVERRUN_TIMES
/*********************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************/
//[7]:0,没有成功的捕获; 1,成功捕获到一次.
//[6]:0,还没捕获到高电平; 1,已经捕获到高电平了.
//[5:0]:捕获高电平后溢出的次数
u8 TIM2_CH1_CAPTURE_STA = 0;    // 输入捕获状态标志
u16 TIM2_CH1_CAPTURE_VAL = 0;   // 输入捕获值
/*********************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************/
/**
 * @brief 通用定时器中断初始化
 *        这里时钟选择为APB1的2倍，而 APB1 为 36M, 所以 Tim2-7 时钟为 72M
 * 		  Tout = ((arr + 1) * (psc + 1)) / Tclk
 * 		  计数器的时钟频率计算: fclk  = fCK_PSC/(psc+1), 定时器主频 / (分频系数 + 1)
 *
 * @param[in]  arr 自动重装值, 范围:0~2^16
 * @param[in]  psc 时钟预分频数
 */
void TIM3_Int_Init(u16 arr, u16 psc)
{
    RCC->APB1ENR |= 1 << 1;		// TIM3时钟使能    
    TIM3->ARR = arr;  			// 设定计数器自动重装值 
    TIM3->PSC = psc;  			// bit[15:0], 预分频器设置
    TIM3->DIER |= 1 << 0;   	// 允许更新中断				
    TIM3->CR1 |= 1 << 0;    	// 使能定时器3
#if NVIC_TEST
    ////抢占1，子优先级3，组2
    MY_NVIC_Init(NVIC_PREEMPTION_PRIORITY_MIDDLE, NVIC_SUBPRIORITY_MIDDLE, 
                TIM3_IRQn, DEFAULT_NVIC_GROUP);
#else
    MY_NVIC_Init(NVIC_PREEMPTION_PRIORITY_MIDDLE, NVIC_SUBPRIORITY_MIDDLE, 
                TIM3_IRQn, DEFAULT_NVIC_GROUP);
#endif
    LOG_D("timer3 init\r\n");
    return;								 
}

/**
 * @brief Timer1 init, 高级定时器
 * 	 	  配置为 PWM 输出模式, 控制 LED0 亮度
 * 		  管脚复用, TIM1_CH1 对应 GPIO PA8， 
 *        PWM 输出频率: 72MHz / ((arr + 1) * (psc + 1))
 * 
 * @param arr 重装载值
 * @param psc 分频系数
 */
void TIM1_PWM_Init(u16 arr, u16 psc)
{
    RCC->APB2ENR |= 1 << 11;	// TIM1 clock enable
    GPIOA->CRH &= 0XFFFFFFF0;   // PA8 清除之前的设置
    GPIOA->CRH |= 0X0000000B;   //复用功能输出 
    TIM1->ARR = arr;            //设定计数器自动重装值
    TIM1->PSC = psc;            //预分频器设置
    TIM1->CCMR1 |= 7 << 4;      // 0111 << 4, CH1 PWM2 模式
    TIM1->CCMR1 |= 1 << 3;      //CH1 预装载使能
    TIM1->CCER |= 1 << 0;       //OC1 输出使能 
    TIM1->BDTR |= 1 << 15;      //MOE 主输出使能 
    TIM1->CR1 = 0x0080;         //ARPE 使能
    TIM1->CR1 |= 0x01;          //使能定时器 1
    LOG_D("timer1 init\r\n");
    return;
}

/**
 * @brief Timer2 输入捕获实验
 *        测量 WK_UP 按键的高电平脉冲宽度, PA0 管脚复用(Timer2 CH1)
 * 
 * @param arr 
 * @param psc 
 */
void TIM2_INPUT_Init(u16 arr, u16 psc)
{
    RCC->APB1ENR |= 1 << 0;     //TIM2 时钟使能
    RCC->APB2ENR |= 1 << 2;     //使能 PORTA 时钟 
    GPIOA->CRL &= 0XFFFFFFF0;   //PA0 清除之前设置 
    GPIOA->CRL |= 0X00000008;   //PA0 输入 
    GPIOA->ODR |= 0 << 0;       //PA0 内部下拉
    TIM2->ARR = arr;            //设定计数器自动重装值 
    TIM2->PSC = psc;            //预分频器
    TIM2->CCMR1 |= 1 << 0;      // bit[1:0] : 01, CC1S=01 选择输入端 IC1 映射到 TI1 上
    TIM2->CCMR1 |= 1 << 4;      //IC1F=0001 配置滤波器 以 Fck_int 采样，2 个事件后有效
    TIM2->CCMR1 |= 0 << 10;     //IC2PS=00 配置输入分频,不分频
    TIM2->CCER |= 0 << 1;       //CC1P=0 上升沿捕获
    TIM2->CCER |= 1 << 0;       //CC1E=1 允许捕获计数器的值到捕获寄存器中
    TIM2->DIER |= 1 << 1;       //允许捕获中断
    TIM2->DIER |= 1 << 0;       //允许更新中断
    TIM2->CR1 |= 0x01;          //使能定时器 2

    //抢占 2，子优先级 0，组 2
    MY_NVIC_Init(NVIC_PREEMPTION_PRIORITY_MIDDLE, TIM2_SUB_RPIORITY,
                TIM2_IRQn, DEFAULT_NVIC_GROUP);
    LOG_D("timer2 init\r\n");
    return;
}

/**
 * @brief 定时器3中断服务程序
 * 
 */
void TIM3_IRQHandler(void)
{
    LOG_D("%s enter %X", __func__, (uint16_t)TIM3->SR);
    if (TIM3->SR & 0X0001) {//溢出中断
#if TIMER3_TEST_ENABLE
        LED1 =! LED1;	
        // LOG_D("%s", __func__);
#endif
#if NVIC_TEST
        // 在定时器中断中 delay， 按键(外部中断)设置不同的优先级,观察能否打断/pending 该中断
        LOG_D("delay");
        //TODO:
        // 这里有个问题, 这个接口并不能 delay 5s
        delay_ms(5000);
#endif
    }
    TIM3->SR &= ~(1 << 0); //清除中断标志位 
    LOG_D("%s exit %X", __func__, (uint16_t)TIM3->SR);

    return;  
}

/**
 * @brief set timer1 reload num
 * 
 * @param val 
 */
void TIM1_set_reload_val(uint32_t val)
{
    TIMER1_CH1_PWM_VAL = val;

    return;
}

/**
 * @brief tiemr2 capture high level
 *        PA0 默认低电平, 捕获到上升沿后开始计时, 下降沿停止计时
 * 
 */
void TIM2_IRQHandler(void)
{
    uint16_t timerStatus = TIM2->SR;

    if((TIM2_CH1_CAPTURE_STA & TIM2_CAPTURE_COMPLETE) == 0){ // 还未成功捕获
        // printf("%02X %d %d %04X\r\n", TIM2_CH1_CAPTURE_STA, TIM2_CH1_CAPTURE_VAL, TIM2->CCR1, timerStatus);

        if(timerStatus & (1 << 0)){ // bit[0] 更新中断时间发生
            if(TIM2_CH1_CAPTURE_STA & TIM2_CAP_HIGH_LEVEL){ // 已经捕获到高定平
                if((TIM2_CH1_CAPTURE_STA & 0x3F) == 0x3F){ // state bit[5:0]高电平时间太长, 超过可以统计的最大次数, 则强制统计成功
                    TIM2_CH1_CAPTURE_STA |= TIM2_CAPTURE_COMPLETE; // 标记已经成功读取到脉冲宽度
                    TIM2_CH1_CAPTURE_VAL = 0xFFFF;
                }
                else{
                    ++TIM2_CH1_CAPTURE_STA;
                }
            }
        }

        if(timerStatus & (1 << 1)){ // bit[1] TIM2_CNT 与 TIM2_CCR1 匹配, 计数值与比较值匹配
            if(TIM2_CH1_CAPTURE_STA & TIM2_CAP_HIGH_LEVEL){ // 已经捕获到上升沿, 又捕获到一个下降沿
                TIM2_CH1_CAPTURE_STA |= TIM2_CAPTURE_COMPLETE;
                TIM2_CH1_CAPTURE_VAL = TIM2->CCR1; // 获取当前的计数值, 超时此时 * 捕获周期 + 当前计数时间 = 脉冲时间
                TIM2->CCER &= ~(1 << 1); // 设置为上升沿捕获
            }
            else{ // 第一次捕获到上升沿
                TIM2_CH1_CAPTURE_VAL = 0; // 清零
                TIM2_CH1_CAPTURE_STA = 0;
                TIM2_CH1_CAPTURE_STA |= TIM2_CAP_HIGH_LEVEL; // 标记捕获到上升沿
                TIM2->CNT = 0; // 计数器清空
                TIM2->CCER |= (1 << 1); // 设置为捕获下降沿
            }
        }
    }

    TIM2->SR = 0; // clear interrupt

    return;
}

