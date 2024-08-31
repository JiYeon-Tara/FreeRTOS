#include "sys.h"

#include "ulog.h"
#include "util.h"


/*********************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************/
/**
 * @brief 一种测试当前使用的芯片支持多少位来表示优先级的方法:
 *        方法:向某个寄存器写入 0xFF，然后读出该寄存器, 获取 bit 为 1 的数目
 *        原理:不支持的位写入 1 后, 无效, 仍然返回 0
 * 
 * @return u8 
 */
u8 NVIC_Get_Chip_Support_Priority_Bit_Num(void)
{
    u8 idx, val, bit1_num = 0;
    
    NVIC->IP[0]=0xFF;
    val = NVIC->IP[0];
    LOG_D("val=%X", val);
    for (idx = 0; idx < 8; ++idx) {
        if ((val >> idx) & 0x01) {
            ++bit1_num;
        }
    }

    NVIC->IP[0] = val; // restore
    LOG_D("chip suppoer priority level bit num:%d", bit1_num);

    return bit1_num;
}
/*********************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************/
/**
 * @brief 设置向量表的基地址以及偏移地址
 * 		  NVIC 的偏移地址
 * 
 * @param NVIC_VectTab  基址
 * @param Offset 偏移量	
 * @return * void 
 */
void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset)	 
{ 	  
    //用于标识向量表是在 CODE 区还是在RAM区 	 
    SCB->VTOR = NVIC_VectTab | (Offset & (u32)0x1FFFFF80); //设置NVIC的向量表偏移寄存器

    return;
}

/**
 * @brief 设置NVIC分组 NVIC_Group,
 *        只需要在开机后设置一次,仅在开机 NVIC 初始化的时候调用一次
 * 
 * @param 设置 NVIC->AIRCR[10:8],参数范围:0~4 
 *      对应 NVIC_Group NVIC 分组 7~3,总共5组(Cortex-M3可以支持8组,STM32仅支持 4 bit), 
 *      STM32 有效位总共 4 位(高4位),有效参数范围:3~7,
        //Cortex-M3组划分:
        //组0:7位抢占优先级 bit[7:1], 1位子优先级 bit[0:0]
        //组1:6位抢占优先级 bit[7:2], 2位子优先级 bit[1:0]
        //组2:6位抢占优先级 bit[7:3], 3位子优先级 bit[2:0]
        //组3:4位抢占优先级 bit[7:4], 4位子优先级 bit[3:0]
        //组4:3位抢占优先级 bit[7:5], 5位子优先级 bit[4:0]
        //组5:2位抢占优先级 bit[7:6], 6位子优先级 bit[5:0]
        //组6:1位抢占优先级 bit[7:7], 7位子优先级 bit[6:0]
        //组7:0位抢占优先级 无, 8位子优先级 bit[7:0]
 * @return void
 */
void NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{
    u32 AIRCR_val, temp1;	

    temp1 = (~NVIC_Group) & 0x07; //仅读取后三位(范围0-7), 等效于 7-NVIC_Group, Q:这里为什么要取反呢？A:高4位有效,但是填入参数范围为0~4

    AIRCR_val = SCB->AIRCR; //读取先前的设置, bit[10:8]
    AIRCR_val &= 0X0000F8FF; //清空先前优先级分组 bit[10:8], 0xF8 = 1111 1000
    AIRCR_val |= 0X05FA0000; //VECTKEY 钥匙
    AIRCR_val |= (temp1 << 8); // NVIC->AIRCR[10:8] 设置新的优先级分组
    SCB->AIRCR = AIRCR_val;  //设置分组	    	  				   

    return;
}

/**
 * @brief 设置 NVIC 中断优先级
 *        NVIC_SubPriority和NVIC_PreemptionPriority的原则是,数值越小,优先级越高
 * 
 * @param NVIC_PreemptionPriority 抢占优先级, 取值范围:0-3
 * @param NVIC_SubPriority 响应优先级, 取值范围:0-3
 * @param NVIC_Channel 中断编号, 对应哪个中断线, 芯片固定的。范围:0-239, Cortex-M3 最多支持 240 个外部中断, STM32支持 68 个
 * @param NVIC_Group 主要为了兼容其他文件中的接口, 否则删除该参数,中断分组 0~4
 *        DEFAULT_NVIC_GROUP 2, 2 位抢占优先级, 2 位子优先级
 * @return * void 
 * @desc: NVIC->IP 寄存器, 每一个中断对应一个 8 位的寄存器, 这个寄存器只有 低4位有效, bit[3:0]:[抢占优先级:子优先级]
 *          //注意优先级不能超过设定的组的范围!否则会有意想不到的错误

 */
void MY_NVIC_Init(u8 NVIC_PreemptionPriority, u8 NVIC_SubPriority, u8 NVIC_Channel, u8 NVIC_Group)	 
{
    u32 temp;

    UNUSED_PARAM(NVIC_Group); // 主要为了兼容其他文件中的接口, 否则删除该参数
    // NVIC_PriorityGroupConfig(NVIC_GROUP); //仅开机初始化 NVIC 设置一次

    temp = NVIC_PreemptionPriority << (4 - NVIC_Group);	  
    temp |= NVIC_SubPriority & (0x0f >> NVIC_Group);
    temp &= 0xf; //取低四位  1111
    NVIC->ISER[NVIC_Channel/32] |= (1 << NVIC_Channel % 32); //使能中断位(要清除的话,相反操作就OK) 
    NVIC->IP[NVIC_Channel] |= temp << 4; //设置抢占优先级和子优先级 STM32 仅高四位有效,bit[7:4] 包括抢占优先级和子优先级

    return; 	  				   
}

/**
 * @brief 外部中断配置函数
 * 		  eg:GPIO_NVIC_Config(PC, pin5, 1);	// GPIO_NVIC_Config(2, 5, 1);
 * 
 * @param GPIOx 0~6,代表GPIOA~G
 * @param BITx  需要使能GPIO中的哪一个端口, 0~15
 * @param trigger_mode  trigger mode, 触发模式:1,下升沿;2,上降沿;3，任意电平触发
 * @return * void 
 * @desc: 只针对GPIOA~G;不包括PVD,RTC和USB唤醒这三个
 * 	  	  该函数一次只能配置1个IO口,多个IO口,需多次调用
 * 		  该函数会自动开启对应中断,以及屏蔽线   
 */
void GPIO_NVIC_Config(u8 GPIOx, u8 BITx, u8 trigger_mode) 
{
    u8 EXTADDR;
    u8 EXTOFFSET;
    EXTADDR = BITx / 4; // 得到中断寄存器组的编号, EXTICR 的编号, EXTICR[4]
    EXTOFFSET = (BITx % 4) * 4; 
    RCC->APB2ENR |= (1 << 0); // 使能io复用时钟，辅助功能 IO 时钟使能
    AFIO->EXTICR[EXTADDR] &= ~(0x000F << EXTOFFSET); //清除原来设置！！！
    AFIO->EXTICR[EXTADDR] |= GPIOx << EXTOFFSET;// EXTI.BITx映射到GPIOx.BITx 
    //自动设置
    EXTI->IMR |= 1 << BITx;//  开启line BITx上的中断
    //EXTI->EMR |= 1<<BITx;//不屏蔽line BITx上的事件 (如果不屏蔽这句,在硬件上是可以的,但是在软件仿真的时候无法进入中断!)

    if (trigger_mode & 0x01) // 0000 0001
        EXTI->FTSR |= 1 << BITx;//line BITx上事件下降沿触发
    if (trigger_mode & 0x02) // 0000 0010
        EXTI->RTSR |= 1 << BITx;//line BITx上事件上升降沿触发
    if (trigger_mode & 0x03){ // 0x03 :0000 0011, 在前面已经配置
        ;						//任意电平触发
    }

    // if(TRIM == 0x01){ // falling trigger
    // 	EXTI->FISR |= 1 << BITx;
    // }
    // else if(TRIM == 0x02){ // rising trigger
    // 	EXTI->RISR |= 1 << BITx;
    // }
    // else if(TRIM == 0x03){ // both edge
    // 	EXTI->FISR |= 1 << BITx;
    // 	EXTI->RISR |= 1 << BITx;
    // }

    return;
}

/**
 * @brief 把所有时钟寄存器复位
 * 不能在这里执行所有外设复位!否则至少引起串口不工作.
 * 
 */
void MYRCC_DeInit(void)
{	
    RCC->APB1RSTR = 0x00000000; // 复位 RCC_AHBRSTR
    RCC->APB2RSTR = 0x00000000; // 复位 APB2RSTR
    RCC->AHBENR = 0x00000014; // 睡眠模式 flash (闪存)和 SRAM 时钟使能, 其他关闭.	  
    RCC->APB2ENR = 0x00000000; //外设时钟关闭.
    RCC->APB1ENR = 0x00000000; // reset
    RCC->CR |= 0x00000001; // 使能内部高速时钟HSION, 系统刚上电的时候使用的是内部高速时钟(HSI), 之后会切换为外部高速时钟(HSE)
    RCC->CFGR &= 0xF8FF0000; // 复位SW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]
    RCC->CR &= 0xFEF6FFFF; // 复位HSEON,CSSON,PLLON
    RCC->CR &= 0xFFFBFFFF; // 复位HSEBYP	   	  
    RCC->CFGR &= 0xFF80FFFF; //复位PLLSRC(0：HSI振荡器时钟经2分频后作为PLL输入时钟), PLLXTPRE, PLLMUL[3:0] and USBPRE 
    RCC->CIR = 0x00000000; //关闭所有中断
    
#ifdef VECT_TAB_RAM
    //配置中断向量表
    MY_NVIC_SetVectorTable(0x20000000, 0x0); // 内部 RAM 区
#else   
    MY_NVIC_SetVectorTable(0x08000000, 0x0); // Code 区, flash 区域
#endif

    return;
}

/**
 * @brief 执行完 WFI 指令以后, STM32 就进入待机模式了, 系统将停止工作, 此时 JTAG 会失效
 * 
 */
void WFI_SET(void)
{
    //THUMB指令不支持 "汇编内联"———— 在 C 语言中直接使用汇编
    //采用如下方法实现执行汇编指令 WFI
    // __ASM volatile("assemble language");
    __ASM volatile("wfi");
}

//关闭所有中断
void INTX_DISABLE(void)
{
    __ASM volatile("cpsid i");
}

//开启所有中断
void INTX_ENABLE(void)
{
    __ASM volatile("cpsie i");		  
}


/**
 * @brief 设置栈顶地址
 * 
 * @param addr 栈顶地址
 * @return NULL 
 */
__asm void MSR_MSP(u32 addr) 
{
    //move special register to register
    //MSP -> R0
    MSR MSP, r0 //set Main Stack value
    BX r14
}

/**
 * @brief 进入待机模式	  
 * 
 */
void Sys_Standby(void)
{
    LOG_I("enter sys standby mode\r\n");
    SCB->SCR |= (1 << 2); // 使能 SLEEPDEEP 位 (SYS->CTRL), 《cortex-M3权威指南》
    // 因为要配置电源控制寄存器，所以必须先使能电源时钟。然后再设置 PWR_CSR 的 EWUP
    // 位，使能 WK_UP 用于将 CPU 从待机模式唤醒。
    RCC->APB1ENR |= (1 << 28); // 使能电源时钟, 
    PWR->CSR |= (1 << 8); // 设置 WKUP 用于唤醒, WKUP引脚用于将CPU从待机模式唤醒，WKUP引脚被强置为输入下拉的配置(WKUP引脚上的上升沿将系统从待机模式唤醒)
    PWR->CR |= (1 << 2); // 清除 Wake-up 标志
    PWR->CR |= (1 << 1); // PDDS置位,PDDS：进入掉电深睡眠
    WFI_SET(); // 执行WFI指令

    return;		 
}

/**
 * @brief 系统软复位, 操作 SCB->AIRCR 实现
 * 
 */
void Sys_Soft_Reset(void)
{   
    SCB->AIRCR = 0X05FA0000 | (u32)0x04;	  //0x05FA 0000 是编辑SCB 的密钥
    //SCB->AIRCR = 0x05FA0000 | (u32)(1<<2);	//bit[2] 产生软件复位

    return;
}


#define JTAG_SWD_DISABLE   0X02
#define SWD_ENABLE         0X01
#define JTAG_SWD_ENABLE    0X00
/**
 * @brief JTAG模式设置,用于设置JTAG的模式
 *        jtag, swd模式设置;
 * 
 * @param mode 00,全使能; 01,使能SWD; 10,全关闭;	  
 */
void JTAG_Set(u8 mode)
{
    u32 temp;
    temp = mode;
    temp <<= 25;
    RCC->APB2ENR |= 1 << 0;     // 开启辅助时钟
    // AFIO->MAPR[26:24], start from 0
    AFIO->MAPR &= 0XF8FFFFFF; 	// 清除MAPR的[26:24]
    AFIO->MAPR |= temp;       	//设置jtag模式

    return;
}

/**
 * @brief 系统时钟初始化函数
 * 
 * @param PLL 选择的倍频数，从2开始，最大值为16		
 */
void Stm32_Clock_Init(u8 PLL)
{
    unsigned char temp=0;   

    MYRCC_DeInit(); // 复位并配置向量表 NVIC
    RCC->CR |= 0x00010000;  // 外部高速时钟使能HSEON, 1 << 16, HSI(RC) ——> HSE(crystal)
    while(!(RCC->CR >> 17))	;//等待外部时钟就绪, HSE RDY flag : bit[17], RCC & (1 << 17)
    RCC->CFGR = 0X00000400; // bit[10:8] 100：HCLK 2分频, APB1=DIV2(二分频); APB2=DIV1(一分频 = 不分频); AHB=DIV1;
    PLL -= 2; //抵消2个单位（因为是从2开始的，设置0就是2), 0010：PLL 4倍频输出
    RCC->CFGR |= PLL << 18; // PLLMUL, bit[21:18] 设置 PLL 值 2~16, 倍频数
    RCC->CFGR |= 1 << 16; // PLLSRC, 1, HSE 作为时钟源 
    FLASH->ACR |= 0x32; // FLASH 2个延时周期
    RCC->CR |= 0x01000000; // PLLON：PLL使能 (PLL enable)
    while(!(RCC->CR >> 25));//等待PLL锁定
        
    RCC->CFGR |= 0x00000002;//PLL作为系统时钟	 
    while (temp != 0x02) { //等待PLL作为系统时钟设置成功, RCC->CFGR[3:2] -> SWS, 0x02 - 使用 PLL 作为系统时钟
        temp = RCC->CFGR >> 2;	//CRGR[3:2], SWS, 表示系统时钟的切换状态, 由硬件set or reset 
        temp &= 0x03; //低两位
    }

    return;
}











