#include "usart.h"	  

////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
//V1.6修改说明 20150109
//uart_init函数去掉了开启PE中断
//V1.7修改说明 20150322
//修改OS_CRITICAL_METHOD宏判断为：SYSTEM_SUPPORT_OS
////////////////////////////////////////////////////////////////////////////////// 	  

// macro
//组2，最低优先级， 抢占优先级 和 子优先级的值都是2
#define UART1_PRIEMPTION_PRIORITY  3
#define UART1_SUB_PRIORITY         3
#define UART1_NVIC_GROUP           2	

#define UART_DEBUG_MAX_BUFF_SIZE			128				// 串口接收 buffer 大小
#define SUPPORT_AT_CMD						1  				// 是否支持 AT 命令


//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
    int handle; 
    /* Whatever you require here. If the only file you are using is */ 
    /* standard output using printf() for debugging, no file handling */ 
    /* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout;   

//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
    x = x; 
} 

// 重定向fputc函数
// printf的输出，指向fputc，由fputc输出到串口
// printf() 还是通过调用 fputc(char ch, 标准输出流)
// 这里使用串口1(USART1)输出printf信息
int fputc(int ch, FILE *f)
{
    while((USART1->SR & 0X40) == 0) // 0100 0000, bit[6], TX complete flag bit
        ;//等待上一次串口数据发送完成  
    USART1->DR = (u8) ch;      	//写DR,串口1将发送数据
    return ch;
}
#endif 


                             

#if EN_USART1_RX   //如果使能了接收
//注意,读取USARTx->SR能避免莫名其妙的错误   	
uint8_t USART_RX_BUF[USART_REC_LEN];     // 接收缓冲,最大USART_REC_LEN个字节.
//接收状态
// bit15，	接收完成标志位
// bit14，	接收到 0x0d 标志位
// bit13~0，	接收到的有效字节数目
uint16_t USART_RX_STA = 0;       //接收状态标记
/**
 * @brief 串口1中断服务程序
 * 
 * @desc: DCD USART1_IRQHandler
 * 		  (1)回车: 0x0D 0x0A;
 * 		  (2)如果有操作系统, 则需要 OSIntEnter(), OSInetExit();
 * 		  (3)在 startup_stm32f10x_hd.s 中有声明;
 */
void USART1_IRQHandler(void)
{
    u8 res;	

// #if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
// 	OSIntEnter();    
// #endif

    if(USART1->SR & (1 << 5))	//SR bit[5], RXNE==1, 接收到数据
    {
        // 从数据寄存器中取出数据
        res = USART1->DR; 		//UART1->DR 只有 bit[8:0] 有效
        //res = USART1->DR & 0xFF;
// #if SUPPORT_AT_CMD
//         // 支持 AT 命令
//         // AT 命令解析 单线程(多线程则发送数据指针到解析线程队列即可)
//         // 结尾必须添加回车换行 AT^START=1=2\r\n
//         // 	static char tempBuff[UART_DEBUG_MAX_BUFF_SIZE];
//         // 	static bufPos = 0;

// #else
        if((USART_RX_STA & 0x8000) == 0)	// bit[15] != 0, 接收未完成, 保存到 USART_RX_BUF 即可
        {
            if(USART_RX_STA & 0x4000)	// bit[14] == 1, 已经接收到了0x0d, 每次接收数据都要 \r\n 结尾, 否则认为错误
            {
                if(res != 0x0A)
                    USART_RX_STA = 0;//接收错误,重新开始
                else 
                    USART_RX_STA |= 0x8000;	//接收完成了， 置位 bit[15]
            }
            else //还没收到0X0D
            {
                if(res == 0x0d)				//当前收到 0x0D
                    USART_RX_STA |= 0x4000;
                else						//还没收到 0x0D, 是其它字节
                {
                    uint16_t recvDataBytes = USART_RX_STA & UART_GET_RX_LEN;
                    // USART_RX_BUF[USART_RX_STA & 0X3FFF] = res;	// bit[13:0], 保存接收到的有效字节数目, 0x3F 0011 1111
                    USART_RX_BUF[recvDataBytes] = res;
                    ++USART_RX_STA;			// 接收到的有效字节数目+1
                    if(USART_RX_STA > (USART_REC_LEN - 1))		// 收到数据太多, 认为错误
                        USART_RX_STA = 0;	// 接收数据错误,重新开始接收
                }		 
            }
        }  	
// #endif // SUPPORT_AT_CMD	 									     
    }

// #if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
// 	OSIntExit();  											 
// #endif

    return;
} 
#endif // end EN_USART1_RX	

/** 
 * @brief 初始化IO 串口1
 * 
 * @param pclk2 PCLK2时钟频率(hz)
 * @param baud 波特率 
 */
void uart1_init(u32 pclk2, u32 baud)
{
    float temp;
    u16 mantissa;
    u16 fraction;	 

    // 根据波特率计算寄存器设置参数 ???
    temp = (float)(pclk2) / (baud * 16);	// 得到USARTDIV
    mantissa = temp;				 		// 得到整数部分
    fraction = (temp-mantissa) * 16; 		// 得到小数部分	 
    mantissa <<= 4;
    mantissa += fraction; 

    RCC->APB2ENR |= 1 << 2;   // 使能PORTA口时钟, TXD(PA9), RXD(PA10)
    RCC->APB2ENR |= 1 << 14;  // 使能串口时钟 RCC->APB2ENR:bit[14]
    GPIOA->CRH &= 0XFFFFF00F; // IO状态清零
    GPIOA->CRH |= 0X000008B0; // IO状态设置 1000(pin10, RX, 模拟输入模式) 1011(pin9, TX, 复用功能推挽输出模式)
    // APB2RSTR(外设复位寄存器)
    RCC->APB2RSTR |= 1 << 14;   // 复位串口1, RCC->APB2RSTR:bit[14], 软件清零以及复位
    RCC->APB2RSTR &= ~(1 << 14);//停止复位	   	   
    //波特率设置
     USART1->BRR = mantissa; // 波特率寄存器设置	 
    USART1->CR1 |= 0X200C;  // 1位停止,无校验位. 0x200C - 0010 0000 0000 1100
    // USART1->CR1 |= (1 << 13);
    // 还有串口发送完成中断, 以及发送缓冲区空中断等其他串口中断可以使用
    // UART1->CR1 , PEIE, TXEIE, TCIE...

#if EN_USART1_RX		  //如果使能了接收
    //使能接收中断 
    USART1->CR1 |= 1 << 5;    //接收缓冲区非空中断使能
    MY_NVIC_Init(UART1_PRIEMPTION_PRIORITY, UART1_SUB_PRIORITY, USART1_IRQn, UART1_NVIC_GROUP);	// 中断线是固定的, 但是中断优先级可以调整
#endif

    return;
}

// TODO:
/**
 * @brief UART_DMA 如何实现 ??
 *  
 */


