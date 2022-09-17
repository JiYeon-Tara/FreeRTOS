#ifndef __DMA_STM_H__
#define __DMA_STM_H__

#ifndef __DMA_H
#define	__DMA_H	   
#include "sys.h"

/*************
 * TYPEDEF
 *************/
typedef struct
{
	uint8_t sendBuff[5168];
	uint8_t textToSend[100];	// 要发送的内容需要小于 100
	uint8_t sendSize;			// 要发送的数据长度

} dma_manager_t;


/*************
 * DECLARATION
 *************/
dma_manager_t dma_manager;

void DMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);//配置DMA1_CHx
void DMA_Enable(DMA_Channel_TypeDef*DMA_CHx);//使能DMA1_CHx

#endif
































#endif