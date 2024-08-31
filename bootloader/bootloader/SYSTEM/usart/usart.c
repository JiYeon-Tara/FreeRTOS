#include "usart.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h" //ucos 使用	  
#endif

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
void _sys_exit(int x) 
{ 
	x = x; 
}

//重定向fputc函数
//printf的输出，指向fputc，由fputc输出到串口
//这里使用串口1(USART1)输出printf信息
int fputc(int ch, FILE *f)
{
	while((USART1->SR & 0X40) == 0);//等待上一次串口数据发送完成  
	USART1->DR = (u8) ch;      	//写DR,串口1将发送数据
	return ch;
}
#endif 
//end


#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误
//接收缓冲,最大USART_REC_LEN个字节,起始地址为0X20001000(SRAM APP 起始地址).
u8 USART_RX_BUF[USART_REC_LEN] __attribute__ ((at(0X20001000)));

//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       	//接收状态标记	  
u16 USART_RX_CNT=0;			//接收的字节数	 
  
void USART1_IRQHandler(void)
{
	u8 res;	
#if SYSTEM_SUPPORT_OS //如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if (USART1->SR & (1 << 5)) {//接收到数据
		res = USART1->DR;
		if (USART_RX_CNT < USART_REC_LEN) {
			USART_RX_BUF[USART_RX_CNT] = res;
			USART_RX_CNT++;
		}
	}
#if SYSTEM_SUPPORT_OS //如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}
#endif

/**
 * @brief 初始化IO 串口1
 * 
 * @param pclk2 PCLK2时钟频率(Mhz)， APB2
 * @param bound 波特率 
 */
void uart_init(u32 pclk2, u32 bound)
{
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp = (float)(pclk2 * 1000000) / (bound * 16);//得到USARTDIV
	mantissa = temp; //得到整数部分
	fraction = (temp - mantissa) * 16; //得到小数部分	 
    mantissa <<= 4;
	mantissa += fraction; 
	RCC->APB2ENR |= 1 << 2; //使能PORTA口时钟  
	RCC->APB2ENR |= 1 << 14; //使能串口时钟 
	GPIOA->CRH &= 0XFFFFF00F; //IO状态设置
	GPIOA->CRH |= 0X000008B0; //IO状态设置 
	RCC->APB2RSTR |= 1 << 14; //复位串口1
	RCC->APB2RSTR &= ~(1 << 14);//停止复位	   	   
	//波特率设置
 	USART1->BRR=mantissa; // 波特率设置	 
	USART1->CR1|=0X200C; //1位停止,无校验位.
#if EN_USART1_RX //如果使能了接收
	//使能接收中断 
	USART1->CR1|=1<<5; //接收缓冲区非空中断使能	    	
	MY_NVIC_Init(3,3,USART1_IRQn,2);//组2，最低优先级 
#endif
}
