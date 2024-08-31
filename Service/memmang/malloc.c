/**
 * @file malloc.c
 * @author your name (you@domain.com)
 * @brief 内存管理实验
 * @version 0.1
 * @date 2022-09-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "malloc.h"
#include "usart.h"

// Questions:
/************************* 这是怎么实现的???????????????????????????? **************************************/
//(1) 如果需要定义大数组, 定义到外部 RAM, 
// #define LOCATE_IN_EXT_RAM    __attribute__(("section...."))

//(2)调试内存管理功能的时候, 程序正在运行的时候按了一下复位键, 板子就跑不了了, 然后必须要重新烧录代码才可以跑一次, 重复开机仍然不可用
// 网上说是: 晶振坏了?????????????
// 开始以为是操作 falsh 把代码区给修改了, 发现好像并不是这个原因

// (3) 如何设置给标准库的 malloc() 分配的内存? 如果使用 malloc() 最大可以分配多少内存?
// malloc() 内存管理策略是什么样子的, 分配效率比较低?

// (4) keil 编译, 内存不够时会打印
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching w25q64.o(.bss).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching stm_flash.o(.bss).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching startup_stm32f10x_hd.o(STACK).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching startup_stm32f10x_hd.o(HEAP).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching service_at.o(.bss).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching usmart_config.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching usart.o(.bss).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching libspace.o(.bss).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching test.o(.bss).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching touch.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching usmart.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching malloc.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching ili93xx.o(.bss).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching rtc.o(.bss).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching remote.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching usart.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching delay.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching ili93xx.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching timer.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching strtok.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching watch_dog_timer.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching w25q64.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching rtc.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching service_at.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching ft5206.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching gt9147.o(.data).
// ..\OBJ\test.axf: Error: L6406E: No space in execution regions with .ANY selector matching ott2001a.o(.data).
// ..\OBJ\test.axf: Error: L6407E: Sections of aggregate size 0x2348 bytes could not fit into .ANY selector(s).


//  我们在 STM32 FLASH 写入的时候，需要一个 1024 字节的 16 位数组，实际上占
// 用了 2K 字节，而这个数组几乎只能给 STMFLASH_Write 一个函数使用，其实这是非常浪费内
// 存的一种做法。———— 非常浪费内存的做法
// 因此需要使用内存管理, 对较大内存使用动态申请的方式


// 分块式内存管理, 由内存池 和 内存管理表组成
/*********************************************************************************
 * MACRO
 *********************************************************************************/
//内存参数设定
#define MEM_BLOCK_SIZE			32 // 内存块大小为32字节
#define MEM_MAX_SIZE			20 * 1024 // 最大管理内存 42K(0xC000)
#define MEM_ALLOC_TABLE_SIZE	MEM_MAX_SIZE/MEM_BLOCK_SIZE // 内存管理表大小
/*********************************************************************************
 * PUBLIC VARIABLES
 *********************************************************************************/
//内存池(4字节对齐)
__align(4) u8 membase[MEM_MAX_SIZE]; // SRAM内存池

//TODO:
//仅测试使用,可以使用其他内存管理算法
//内存管理表
u16 memmapbase[MEM_ALLOC_TABLE_SIZE]; // SRAM内存池MAP,
//内存管理参数
const static u32 memtblsize = MEM_ALLOC_TABLE_SIZE; // 内存表大小 42K / 32 byte, 每块内存都对应一个内存表, 实际上就是一个标志位
const static u32 memblksize = MEM_BLOCK_SIZE; // 内存块大小, 32 byte
const static u32 memsize = MEM_MAX_SIZE; // 内存总大小:42K


//内存管理控制器
// struct _m_mallco_dev mallco_dev = {
//     .init = mem_init; // 内存初始化
//     .perused = mem_perused; // percent used, 内存使用率
//     .membase = membase, // 内存池
//     .memmap = memmapbase, // 内存管理状态表, 0 表示未被使用; 1 - 表示已经被分配
//     .memrdy = 0, // 内存管理未就绪
// };
struct _m_mallco_dev mallco_dev = {
    mem_init, // 内存初始化
    mem_perused, // percent used, 内存使用率
    membase, // 内存池
    memmapbase, // 内存管理状态表, 0 表示未被使用; 1 - 表示已经被分配
    0 // 内存管理未就绪
};
/*********************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************/
/**
 * @brief 内存管理初始化
 *        初始化内存管理器(全局变量)
 * 
 */
void mem_init(void)  
{
    mymemset(mallco_dev.memmap, 0, memtblsize * 2);//内存状态表数据清零  
    mymemset(mallco_dev.membase, 0, memsize);	//内存池所有数据清零  
    mallco_dev.memrdy = 1;						//内存管理初始化OK  
    LOG_I("memory management module init");
    LOG_I("memory pool address:%X~%X ", (uint32_t)mallco_dev.membase,
            (uint32_t)&mallco_dev.membase[MEM_MAX_SIZE - 1]);
    LOG_I("memory pool info. total size:%d(Byte) = block size:%d(Byte) * block num:%d", 
            MEM_MAX_SIZE, MEM_BLOCK_SIZE, MEM_ALLOC_TABLE_SIZE);
    LOG_I("memory pool info. total size:%d(KB) = block size:%d(Byte) * block num:%d", 
            MEM_MAX_SIZE/1024, MEM_BLOCK_SIZE, MEM_ALLOC_TABLE_SIZE);
    return;
}

/**
 * @brief 获取内存使用率
 * 
 * @return u8 使用率(0~100)
 */
u8 mem_perused(void)  
{
    u32 used = 0;  
    u32 i;  
    for(i = 0; i < memtblsize; i++)   // 直接遍历内存管理表
    {  
        if(mallco_dev.memmap[i])
            used++; 
    }
    return (used * 100) / (memtblsize);  
}

/**
 * @brief 复制内存
 * 
 * @param des 目的地址
 * @param src 源地址
 * @param n 需要复制的内存长度(字节为单位)
 */
void mymemcpy(void *des, void *src, u32 n)  
{
    u8 *xdes = des;
    u8 *xsrc = src; 
    while(n--)
        *xdes++ = *xsrc++; // 一元运算符, 从右往左计算 

    return; 
}


/**
 * @brief 设置内存
 * 
 * @param s 内存首地址
 * @param c 要设置的值
 * @param count 需要设置的内存大小(字节为单位)
 */
void mymemset(void *s, u8 c, u32 count)  
{
    u8 *xs = s;  
    while(count--)
        *xs++ = c;  
    return;
}

/**
 * @brief 内存分配(内部调用)
 *        所属内存块, 内存池就是一个 44K byte 的数组, 所以使用 offset 来管理
 * 
 * @param size u32 要分配的内存大小(字节)
 * @return 0XFFFFFFFF,代表错误; 其他,内存偏移地址 
 *         返回偏移地址, 从内存池首地址开始计算的偏移地址
 *         其实就是内存池数组索引, 大小:0 - MEM_MAX_SIZE - 1
 *         从 offset * memblksize 开始的 size (bytes) 都是本次分配的内存
 *         
 *         内存分配以后, 内存管理表中相应的位置都赋值为 nmemb, 即本次分配占用的内存块的数量
 */
static u32 mem_malloc(u32 size)  
{
    signed long offset = 0;  
    u16 nmemb; // 需要的内存块数, 每一块是 4 byte
    u16 cmemb = 0; // 连续空内存块数
    u32 i;  

    if(!mallco_dev.memrdy) //未初始化,先执行初始化 
        mallco_dev.init();	

    if(size == 0) //不需要分配
        return 0XFFFFFFFF; // NULL 

    nmemb = size / memblksize; //获取需要分配的连续内存块数
    if(size % memblksize) // 如果要分配的内存大小不是 4 byte 对齐的, 也强行分配为 4 byte 对齐
        nmemb++; // 分配内存不能被 block size 整除, 增加一块内存(向上取整)

    // 从末尾开始遍历 memmap
    for(offset = memtblsize - 1; offset >= 0; offset--)	// 遍历内存管理表,找到连续的复合大小的内存块，从末尾开始遍历
    {
        if(!mallco_dev.memmap[offset])//连续空内存块数增加
            cmemb++;	
        else 
            cmemb=0; //连续内存块清零, 大小不满足需要的大小, 就重新遍历新的地址快

        if(cmemb == nmemb) //找到了连续nmemb个空内存块
        {
            for(i = 0; i < nmemb; i++)  				
            {  
                // 这里是不是有 bug??? 连续分配相同大小的内存?
                mallco_dev.memmap[offset + i] = nmemb; //标注内存块非空, 同时分配的内存, 标记相同的数字, 
            }
            return (offset * memblksize); //返回偏移地址, 从内存池首地址开始计算的偏移地址
        }
    }
 
    return 0XFFFFFFFF;//未找到符合分配条件的内存块, 返回 NULL
}

/**
 * @brief 释放内存(内部调用) 
 * 
 * @param offset 内存地址偏移(已知 4 byte 对齐)
 * @return u8 0,释放成功;1,释放失败;  
 */
static u8 mem_free(u32 offset)  
{
    int i;  
    int index;
    int nmemb; // need memory block

    if(!mallco_dev.memrdy)//未初始化,先执行初始化
    {
        mallco_dev.init();    
        return 1;//未初始化  
    }

    if(offset >= memsize){
        return 2;   //偏移超区了.  
    }

    index = offset / memblksize;		//偏移所在内存块号码  
    nmemb = mallco_dev.memmap[index];	//内存块数量
    for(i = 0; i < nmemb; i++)  				
    {
        mallco_dev.memmap[index + i] = 0;   //内存块清零
    }
    return 0;  

    // if(offset < memsize)//偏移在内存池内. 
    // {  
    //     int index=offset/memblksize;		//偏移所在内存块号码  
    //     int nmemb=mallco_dev.memmap[index];	//内存块数量
    //     for(i=0;i<nmemb;i++)  				//内存块清零
    //     {  
    //         mallco_dev.memmap[index+i]=0;  
    //     }
    //     return 0;  
    // }
    // else 
    //     return 2;//偏移超区了.  
}
/*********************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************/
/**
 * @brief 分配内存(外部调用)
 * 
 * @param size 要分配的内存大小(字节)
 * @return void* 分配到的内存首地址. 一定在管理的内存池范围内
 */
void *mymalloc(u32 size)  
{
    u32 offset;  	
                                      
    offset = mem_malloc(size);  	   				   
    if(offset == 0XFFFFFFFF)
        return NULL;  

    // 数组首地址 + 偏移量 -> 实际分配的内存地址
    return (void*)((u32)mallco_dev.membase + offset);  
}

/**
 * @brief 释放内存(外部调用) 
 * 
 * @param ptr 内存首地址, 指针
 */
void myfree(void *ptr)  
{
    u32 offset;

    if(ptr == NULL) //地址为 NULL
        return;
    offset = (u32)ptr - (u32)mallco_dev.membase; // 计算偏移量
    mem_free(offset);	//释放内存     

    return;
}

/**
 * @brief 重新分配内存(外部调用)
 * 
 * @param ptr 旧内存首地址
 * @param size 要分配的内存大小(字节)
 * @return void* 新分配到的内存首地址; 0XFFFFFFFF:分配失败
 */
void *myrealloc(void *ptr, u32 size)  
{
    u32 offset;  

    // 先分配新内存
    offset = mem_malloc(size);

    if(offset == 0XFFFFFFFF) // 分配失败
        return NULL; 
    
    // 拷贝数据到新地址
    mymemcpy((void*)((u32)mallco_dev.membase + offset), ptr, size);	//拷贝旧内存内容到新内存  

    myfree(ptr); //释放旧内存

    return (void*)((u32)mallco_dev.membase + offset); //返回新内存首地址
    // if(offset==0XFFFFFFFF)
        // return NULL;     
    // else  
    // {  									   
    //     mymemcpy((void*)((u32)mallco_dev.membase+offset),ptr,size);	//拷贝旧内存内容到新内存   
    //     myfree(ptr);  											  	//释放旧内存
    //     return (void*)((u32)mallco_dev.membase+offset);  			//返回新内存首地址
    // }  
}


