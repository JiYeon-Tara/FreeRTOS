#include "dma_stm.h"
#include "delay.h"
#include <stdbool.h>

/***/
// DMA - Direcct Memory Access
// 无需 CPU 直接传输控制, 也没有中断处理方式保留现场和恢复现场的过程, 通过硬件为 RAM 与 I/O 设备开辟一条通过, 使得 CPU 的效率大大增加
// 每个通道专门永安里管理来自一个或者多个外设对存储器访问的请求


// UART1 TX - DMA1-ch4
// UART1 RX - DMA1-ch5
// DMA 和串口通信方式一样, 也会有串口发送完成标志以及发送失败等中断标志


u16 DMA1_MEM_LEN;//保存DMA每次数据传送的长度

/**
 * @brief DMA1 的各通道配置
 *        这里的传输形式是固定的,这点要根据不同的情况来修改
 *        从存储器->外设模式/8位数据宽度/存储器增量模式
 * 	      数据方向：存储器 -> 外设
 * 
 * @param DMA_CHx DMA通道CHx
 * @param cpar 外设地址
 * @param cmar 存储器地址
 * @param cndtr 数据传输量  
 */
void DMA_Config(DMA_Channel_TypeDef *DMA_CHx, u32 cpar, u32 cmar, u16 cndtr)
{
	RCC->AHBENR |= 1 << 0;			//开启DMA1时钟
	delay_ms(5);				    //等待DMA时钟稳定

	DMA_CHx->CPAR = cpar; 	 	    //DMA1 外设地址 
	DMA_CHx->CMAR = (u32)cmar; 	    //DMA1,存储器地址

	DMA1_MEM_LEN = cndtr;      	    // 保存DMA传输数据量, 全局变量保存 DMA 的传输数据量

	DMA_CHx->CNDTR = cndtr;    	    //DMA1,传输数据量， 该寄存器的值在 DMA 启动后自减, 每次心动的 DMA 传输, 都重新向该寄存器吸入要传输的数据量
	DMA_CHx->CCR = 0X00000000;	    //复位
	DMA_CHx->CCR |= 1 << 4;  		//从存储器读（存储器 -> 外设）
	DMA_CHx->CCR |= 0 << 5;  		// 普通模式(不执行循环操作)
	DMA_CHx->CCR |= 0 << 6; 		// 外设地址非增量模式(不执行外设地址增量操作)
	DMA_CHx->CCR |= 1 << 7; 	 	//存储器增量模式(执行存储器地址增量操作)
	DMA_CHx->CCR |= 0 << 8; 	 	//外设数据宽度为8位
	DMA_CHx->CCR |= 0 << 10; 		//存储器数据宽度8位
	DMA_CHx->CCR |= 1 << 12; 		//中等优先级
	DMA_CHx->CCR |= 0 << 14; 		//非存储器到存储器模式

	return;		  	
} 


//开启一次DMA传输
void DMA_Enable(DMA_Channel_TypeDef *DMA_CHx)
{
	DMA_CHx->CCR &= ~(1 << 0);       // 关闭DMA传输 
	DMA_CHx->CNDTR = DMA1_MEM_LEN; 	 // DMA1,传输数据量 
	DMA_CHx->CCR |= 1 << 0;          // 开启DMA传输

	return;
}




// 面向对象编程
// dma_manager_t dma_manager;

bool uart1_dma_send(uint8_t *pBuf, uint16_t len)
{

}

bool uart1_dma_recv(uint8_t *pBuf, uint16_t len)
{

}

























