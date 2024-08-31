#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "stmflash.h"
#include "iap.h"


iapfun jump2app; 
u16 iapbuf[1024];

// 将收到的数据爆粗才能到片内 flash 区域
//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
void iap_write_appbin(u32 appxaddr, u8 *appbuf, u32 appsize)
{
    u16 t;
    u16 already_write_half_word=0;
    u16 temp;
    u32 fwaddr = appxaddr;//当前写入的地址
    u8 *dfu = appbuf;

    for(t = 0; t < appsize; t += 2)
    {
        temp=(u16)dfu[1]<<8;
        temp+=(u16)dfu[0];	  
        dfu+=2;//偏移2个字节
        iapbuf[already_write_half_word++]=temp;	    
        if(already_write_half_word == 1024) // 每 2KB, 进行一次写 flash 操作
        {
            already_write_half_word=0;
            STMFLASH_Write(fwaddr, iapbuf, 1024);	
            fwaddr+=2048;//偏移2048  16=2*8.所以要乘以2.
            printf("write addr:%#X size:%dKB success\r\n", fwaddr, 2);
        }
    }

    if(already_write_half_word) { // last block, i!=0 && i!=1024
        STMFLASH_Write(fwaddr, iapbuf, already_write_half_word);//将最后的一些内容字节写进去.
        printf("write addr:%#X size:%fKB success\r\n", fwaddr, 
            (float)already_write_half_word * 2 / 1024);
    }
}

//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
    if (((*(vu32*)appxaddr) & 0x2FFE0000) == 0x20000000) { //检查栈顶地址是否合法.
        //TODO:
        // Q:中断向量表的偏移地址在哪里修改的??????
        // A: MY_NVIC_SetVectorTable(0x08000000,0x8000);
        jump2app=(iapfun)*(vu32*)(appxaddr+4); //用户代码区第二个字为程序开始地址(复位地址)		
        MSR_MSP(*(vu32*)appxaddr); //初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址 MSP)
        jump2app(); //跳转到APP. 相当于修改 PC 为复位地址
    } else {
        printf("MSP invalid:%#X\r\n", *((vu32*)appxaddr));
    }
}














