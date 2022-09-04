#include "watch_dog_timer.h"
#include "led.h"

// 避免程序跑飞
// 独立看门狗由内部专门的 40KHz 低速时钟驱动, 即使主时钟故障, 也依然有效
// 内部 RC 时钟, 时钟不是精确地 40KHz
// 看门狗定时周期通过重装载值 和 分频系数来确定
// Tout = (4 * 2^prer) * rlr / 40
// Tout = (4 * 2^4) * 625 / 40 = 1000 ms
// 由于看门狗的时钟不精确, 所以可能导致时间不那么精确

/*************
 * MACRO
 *************/
#define IWDT_START						0xCCCC		// start watch dog timer
#define IWDT_RELOAD_DATA				0xAAAA		// IWDG->KR, 重装载操作也会启动 IWDG_PR, IWDG_RLR 的写保护
#define IWDT_CLOSE_REG_WRITE_PROTECT	0x5555		// 关闭 IWDG_PR(预分频寄存器), IWDG_RLR 寄存器的写保护

/**
 * @brief 初始化独立看门狗
 *        分频因子=4*2^prer.但最大值只能是256!通过分频因子计算预分频系数
 *        时间计算(大概):Tout=((4*2^prer)*rlr)/40 (ms).
 * @param[in]  prer 分频数:0~7(只有低3位有效!)
 * @param[out] rlr rlr:重装载寄存器值:低11位有效.
 */
void IWDG_Init(u8 prer, u16 rlr) 
{
	IWDG->KR = IWDT_CLOSE_REG_WRITE_PROTECT;	//使能对IWDG->PR和IWDG->RLR的写, close write protect
  	IWDG->PR = prer;  				// 设置分频因子 -> 计算出分频系数   
  	IWDG->RLR = rlr;  				// 从加载寄存器 IWDG->RLR  
	IWDG->KR = IWDT_RELOAD_DATA; 	// reload, 更新初值
  	IWDG->KR = IWDT_START;			// 使能看门狗	
}

/**
 * @brief 喂独立看门狗
 * 
 */
void IWDG_Feed(void)
{
	IWDG->KR = 0XAAAA;	//reload, 自动装载
}


/************************************** 窗口看门狗 **************************************/
// 普通的定时器只要在一个时间点之前对寄存器的值进行重新装载就可以, 
// 窗口定时器要求刷新区间在一个范围内，对之前的范围也进行了限制
// 通常被用来监测由外部干扰或不可预见的逻辑条件造成的应用程序背离正常的运行序列而产生的软件故障
// 避免程序跑飞
//保存WWDG计数器的设置值,默认为最大. 
u8 WWDG_CNT = 0x7f; 

/**
 * @brief 初始化窗口看门狗 	
 *        fprer:分频系数（WDGTB）,仅最低2位有效 
 *        Fwwdg=PCLK1/(4096*2^fprer). 
 * @param[in]  tr T[6:0],计数器值 
 * @param[out] wr 
 * @param fprer W[6:0],窗口值 
 */
void WWDG_Init(u8 tr,u8 wr,u8 fprer) 
{    
	RCC->APB1ENR |= (1 << 11); 	//使能wwdg时钟 
	WWDG_CNT = (tr & WWDG_CNT);   //初始化WWDG_CNT.     
	WWDG->CFR |= fprer<<7;    //PCLK1/4096再除2^fprer 
	WWDG->CFR &= 0XFF80;      
	WWDG->CFR |= wr;     		//设定窗口值      
	WWDG->CR |= WWDG_CNT; 	//设定计数器值 
	WWDG->CR |= (1 << 7);  		//开启看门狗      
	MY_NVIC_Init(WWDT_PRIEMPTION_PRIORITY, WWDT_SUB_PRIORITY, WWDG_IRQn, WWDT_NVIC_GROUP);//抢占2，子优先级3，组2     
	WWDG->SR = 0X00; 			//清除提前唤醒中断标志位 
	WWDG->CFR |= (1 << 9);        //使能提前唤醒中断 
} 

//重设置WWDG计数器的值 
void WWDG_Set_Counter(u8 cnt) 
{
	WWDG->CR =(cnt & 0x7F);//重设置7位计数器 
}

/**
 * @brief 窗口看门狗的中断服务函数 -> 通过窗口看门狗可以实现多个看门狗？
 * 
 */
void WWDG_IRQHandler(void)
{
    printf("%s\r\n", __func__); //不可以在中断里面调这种可能导致延时的函数, 尽量只是设置标志位
	WWDG_Set_Counter(WWDG_CNT);	//重设窗口看门狗的值!         
	WWDG->SR = 0X00;			//清除提前唤醒中断标志位 
	LED1 =! LED1; 
    return;
}
