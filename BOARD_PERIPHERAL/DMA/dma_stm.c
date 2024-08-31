/**
 * @file dma_stm.c
 * @author your name (you@domain.com)
 * @brief DMA 实现串口数据传输
 * @version 0.1
 * @date 2022-12-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "dma_stm.h"
#include "delay.h"
#include <stdbool.h>
#include "board_config.h"
#include "ulog.h"


/***/
// DMA - Direcct Memory Access
// 无需 CPU 直接传输控制, 也没有中断处理方式保留现场和恢复现场的过程, 通过硬件为 RAM 与 I/O 设备开辟一条通过, 使得 CPU 的效率大大增加
// 每个通道专门用来管理来自一个或者多个外设对存储器访问的请求
// ADC, Timer, USART, I2C, I2S, SPI.. 外设 <-> 存储器(内存)


// UART1 TX - DMA1-ch4
// UART1 RX - DMA1-ch5
// DMA 和串口通信方式一样, 也会有串口发送完成标志以及发送失败等中断标志


// TODO:
//配置了 DMA 中断导致 crash
#define DMA_INT_ENABLE 0 // 使能 DMA 中断

static u16 s_dma_mem_len;//保存DMA每次数据传送的长度

/**
 * @brief DMA1 的各通道配置
 *        这里的传输形式是固定的,这点要根据不同的情况来修改
 *        从存储器->外设模式/8位数据宽度/存储器增量模式, 发送数据
 * 	      数据方向：存储器 -> 外设, 这里对应 UART TX
 * 
 * @param DMA_CHx DMA 通道 CHx
 * @param cpar peripheral address外设地址
 * @param cmar 存储器地址
 * @param cndtr 数据传输量  
 */
void DMA_Config(DMA_Channel_TypeDef *DMA_CHx, u32 cpar, u32 cmar, u16 cndtr)
{
    LOG_I("%s enter", __func__);
    RCC->AHBENR |= 1 << 0; //开启DMA1时钟
    delay_ms(5); //等待DMA时钟稳定

    DMA_CHx->CPAR = cpar; //DMA1 外设地址 
    DMA_CHx->CMAR = (u32)cmar; //DMA1,存储器地址
    s_dma_mem_len = cndtr; // 保存DMA传输数据量, 全局变量保存 DMA 的传输数据量
    DMA_CHx->CNDTR = cndtr; // DMA1,传输数据量， 该寄存器的值在 DMA 启动后自减, 每次新的 DMA 传输, 都重新向该寄存器写入要传输的数据量, 因为没有设置循环模式
    DMA_CHx->CCR = 0X00000000; // clear
    DMA_CHx->CCR |= 1 << 4; // 从存储器读（存储器 -> 外设）
    DMA_CHx->CCR |= 0 << 5; // 普通模式(不执行循环操作), 仅进行一次 DMA 传输, 不是每次都是, 这里可能需要更改
    DMA_CHx->CCR |= 0 << 6; // 外设地址非增量模式(不执行外设地址增量操作), 外设地址不会自增, 这里也需要更改
    DMA_CHx->CCR |= 1 << 7; // 存储器增量模式(执行存储器地址增量操作), 存储器地址自增
    DMA_CHx->CCR |= 0 << 8; // 外设数据宽度为8位
    DMA_CHx->CCR |= 0 << 10; // 存储器数据宽度8位
    DMA_CHx->CCR |= 1 << 12; // 中等优先级
    DMA_CHx->CCR |= 0 << 14; // 非存储器到存储器模式

#if DMA_INT_ENABLE //如果使能了 DMA 中断
    DMA_CHx->CCR |= (0x07 << 1); // CCR[3:1] 是否允许半传输中断, 传输完成中断, 传输错误中断
    MY_NVIC_Init(3, 3, DMA1_Channel4_IRQn, 2);	// 中断线是固定的, 但是中断优先级可以调整
#endif

    LOG_I("%s exit", __func__);

    return;		  	
}

/**
 * @brief 开启一次DMA传输
 *        该函数每执行一次, DMA 就发送一次, memory -> peripheral(USART)
 * 
 * @param DMA_CHx 
 */
void DMA_Enable(DMA_Channel_TypeDef *DMA_CHx)
{
    DMA_CHx->CCR &= ~(1 << 0); // 关闭DMA传输 
    DMA_CHx->CNDTR = s_dma_mem_len; // DMA1,传输数据量 
    DMA_CHx->CCR |= 1 << 0; // 开启 DMA 传输

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

int interrupt_enter_cnt = 0;
void DMA1_Channel1_IRQHandler(void)
{
    ++interrupt_enter_cnt;
}

void DMA1_Channel2_IRQHandler(void)
{
    ++interrupt_enter_cnt;
}

void DMA1_Channel3_IRQHandler(void)
{
    ++interrupt_enter_cnt;
}

void DMA1_Channel4_IRQHandler(void)
{
    ++interrupt_enter_cnt;
}

void DMA1_Channel5_IRQHandler(void)
{
    ++interrupt_enter_cnt;
}

void DMA1_Channel6_IRQHandler(void)
{
    ++interrupt_enter_cnt;
}

void DMA1_Channel7_IRQHandler(void)
{
    ++interrupt_enter_cnt;
}


























