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

/************************* 这是怎么实现的???????????????????????????? **************************************/
//(1) 如果需要定义大数组, 定义到外部 RAM, 
// #define LOCATE_IN_EXT_RAM    __attribute__(("section...."))

//(2)调试内存管理功能的时候, 程序正在运行的时候按了一下复位键, 板子就跑不了了, 然后必须要重新烧录代码才可以跑一次, 重复开机仍然不可用
// 网上说是: 晶振坏了?????????????
// 开始以为是操作 falsh 把代码区给修改了, 发现好像并不是这个原因


#if MALLOC_TEST_ENABLE


// 分块式内存管理, 由内存池 和 内存管理表组成

//内存池(4字节对齐)
__align(4) u8 membase[MEM_MAX_SIZE];			// SRAM内存池
//内存管理表
u16 memmapbase[MEM_ALLOC_TABLE_SIZE];			//SRAM内存池MAP
//内存管理参数	   
const u32 memtblsize = MEM_ALLOC_TABLE_SIZE;		//内存表大小 42K / 32 byte
const u32 memblksize = MEM_BLOCK_SIZE;			    //内存分块大小, 32 byte
const u32 memsize = MEM_MAX_SIZE;					//内存总大小:42K


//内存管理控制器
struct _m_mallco_dev mallco_dev =
{
	mem_init,			//内存初始化
	mem_perused,		//内存使用率
	membase,			//内存池
	memmapbase,			//内存管理状态表, 0 表示未被使用; 1 - 表示已经被分配
	0,  				//内存管理未就绪
};


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

//内存管理初始化  
void mem_init(void)  
{  
    mymemset(mallco_dev.memmap, 0, memtblsize * 2);//内存状态表数据清零  
	mymemset(mallco_dev.membase, 0, memsize);	//内存池所有数据清零  
	mallco_dev.memrdy = 1;						//内存管理初始化OK  
    printf("memory management init, start address:%d\n", (uint32_t)mallco_dev.membase);
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

//
//memx:
//size:
//返回值:
/**
 * @brief 内存分配(内部调用)
 *        所属内存块, 内存池就是一个 44K byte 的数组, 所以使用 offset 来管理
 * 
 * @param size u32 要分配的内存大小(字节)
 * @return 0XFFFFFFFF,代表错误; 其他,内存偏移地址 
 */
static u32 mem_malloc(u32 size)  
{  
    signed long offset = 0;  
    u16 nmemb;	//需要的内存块数, 每一块是 4 byte
	u16 cmemb = 0;//连续空内存块数
    u32 i;  

    if(!mallco_dev.memrdy) //未初始化,先执行初始化 
        mallco_dev.init();	
    if(size == 0)
        return 0XFFFFFFFF;				//不需要分配
    nmemb = size / memblksize;  					//获取需要分配的连续内存块数
    if(size % memblksize) // 如果要分配的内存大小不是 4 byte 对齐的, 也强行分配为 4 byte 对齐
        nmemb++;  

    for(offset = memtblsize - 1; offset >= 0; offset--)	//搜索整个内存控制区，从末尾开始遍历
    {
		if(!mallco_dev.memmap[offset])//连续空内存块数增加
            cmemb++;	
		else 
            cmemb=0; //连续内存块清零, 主要大小不满足需要的大小, 就重新遍历新的地址快

		if(cmemb == nmemb) //找到了连续nmemb个空内存块
		{
            for(i = 0; i < nmemb; i++)  				
            {  
                mallco_dev.memmap[offset + i] = nmemb;  //标注内存块非空, 同时分配的内存, 标记相同的数字, 这里是不是有 bug??? 连续分配相同大小的内存?
            }  
            return (offset * memblksize); //返回偏移地址, 从内存池首地址开始计算的偏移地址
		}
    } 
 
    return 0XFFFFFFFF;//未找到符合分配条件的内存块  
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

/**
 * @brief 释放内存(外部调用) 
 * 
 * @param ptr 内存首地址, 指针
 */
void myfree(void *ptr)  
{
	u32 offset;

    if(ptr == NULL)
        return;//地址为0.  
 	offset = (u32)ptr - (u32)mallco_dev.membase; // 计算偏移量
    mem_free(offset);	//释放内存     

    return;
}  

/**
 * @brief 分配内存(外部调用)
 * 
 * @param size 内存大小(字节)
 * @return void* 分配到的内存首地址.
 */
void *mymalloc(u32 size)  
{  
    u32 offset;  	
								      
	offset = mem_malloc(size);  	   				   
    if(offset == 0XFFFFFFFF)
        return NULL;  
    else 
        return (void*)((u32)mallco_dev.membase + offset);  
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
    // 释放原来的内存
    myfree(ptr);  											  	//释放旧内存
    return (void*)((u32)mallco_dev.membase + offset);  			//返回新内存首地址
    // else  
    // {								   
	//     mymemcpy((void*)((u32)mallco_dev.membase + offset), ptr, size);	//拷贝旧内存内容到新内存   
    //     myfree(ptr);  											  	//释放旧内存
    //     return (void*)((u32)mallco_dev.membase+offset);  			//返回新内存首地址
    // }  

}

#endif