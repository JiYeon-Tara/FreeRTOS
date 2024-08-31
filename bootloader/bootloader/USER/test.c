#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"   	 
#include "key.h"	   	 	 
//#include "lcd.h" 
#include "ILI93xx.h"
#include "stmflash.h"	
#include "iap.h"	 


//FLASH分区: 
//0X0800 0000~0x0800 8000,给IAP使用,共32KB字节,
//FLASH APP还有256-32=224KB可用.

// SRAM分区: 
//0X2000 1000处开始,用于存放SRAM IAP代码,
//共44K字节可用,用户可以自行对44K空间进行ROM和RAM区的分配
//特别注意，SRAM APP的ROM占用量不能大于41K字节，因为本例程最大是一次接收41K字节，不能超过这个限制。


int main(void)
{
    u8 t;
    u8 key;
    u16 oldcount=0;	//老的串口接收数据值
    u16 applenth=0;	//接收到的app代码长度
    u8 clearflag=0;  
    Stm32_Clock_Init(9); //系统时钟设置
    uart_init(72,2000000); //串口初始化为256000
    delay_init(72); //延时初始化 
    // LED_Init(); //初始化与LED连接的硬件接口
    // LCD_Init(); //初始化LCD
    KEY_Init(); //按键初始化

    // POINT_COLOR = RED;//设置字体为红色 
    // LCD_ShowString(60,50,200,16,16,"Bootloader");	
    // LCD_ShowString(60,70,200,16,16,"IAP TEST");	
    // POINT_COLOR = BLUE;//设置字体为蓝色
	printf("\r\n\r\n\r\n\r\n\r\n\r\n");
    printf("***************************************\r\n");
    printf("*bootloader start\n");
    printf("***************************************\n\n");

    while (1) {
        delay_ms(20);
        if (USART_RX_CNT) {
            //新周期内,没有收到任何数据,认为本次数据接收完成.
            // TODO: 最少应该有个交互协议
            if (oldcount == USART_RX_CNT) {
                applenth = USART_RX_CNT;
                oldcount = 0;
                USART_RX_CNT = 0;
                printf("Firmware receive finished!\r\n");
                printf("code size:%d Bytes\r\n", applenth);
            } else
                oldcount = USART_RX_CNT;			
        }
        t++;
        //TODO:
        // 因为 通过 USART_RX_CNT 这个变量判断是否接收结束, 需要保证串口发送的频率大于该值
        // 还要保证不丢包, 因为没有校验和重传
#if 0 // LED
        if(t==30)
        {
            LED0=!LED0;
            t=0;
            if(clearflag)
            {
                clearflag--;
                if(clearflag==0)LCD_Fill(60,210,240,210+16,WHITE);//清除显示
            }
        }	  
#endif
        key = KEY_Scan(0);
        if (key == WKUP_PRES) { //WK_UP按键按下, 将接收到的 bin 文件写入到 flash
            if(applenth)
            {
                printf("Start update firmware, write flash\r\n");	
                // LCD_ShowString(60,210,200,16,16,"Copying APP2FLASH...");
                if(((*(vu32*)(0X20001000+4)) & 0xFF000000) == 0x08000000)//判断是否为0X08XXXXXX.
                {
                    iap_write_appbin(FLASH_APP1_ADDR, USART_RX_BUF, applenth);//更新FLASH代码   
                    // LCD_ShowString(60,210,200,16,16,"Copy APP Successed!!");
                    printf("flash write success!\r\n");	
                } else {
                    // LCD_ShowString(60,210,200,16,16,"Illegal FLASH APP!  ");	   
                    printf("illegal flash APP!\r\n");
                }
            } else {
                printf("No APP to update!\r\n");
                // LCD_ShowString(60,210,200,16,16,"No APP!");
            }
            clearflag = 7;//标志更新了显示,并且设置7*300ms后清除显示									 
        }
        if (key == KEY1_PRES) {
            printf("Start exeute Flash App\r\n");
            if(((*(vu32*)(FLASH_APP1_ADDR+4)) & 0xFF000000) == 0x08000000)//判断是否为0X08XXXXXX.
            {
                iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码
            } else {
                printf("Illegal flash APP!\r\n");
                // LCD_ShowString(60,210,200,16,16,"Illegal FLASH APP!");	   
            }
            clearflag=7;//标志更新了显示,并且设置7*300ms后清除显示	  
        }
        if (key == KEY0_PRES) {
            printf("Start execute SRAM APP\r\n");
            // 直接使用串口文件定义的数组中的内容执行
            // "u8 USART_RX_BUF[USART_REC_LEN] __attribute__ ((at(0X20001000)));"
            if (((*(vu32*)(0X20001000+4))&0xFF000000)==0x20000000) { //判断是否为0X20XXXXXX.
                iap_load_app(0X20001000);//SRAM地址
            } else {
                printf("illegal SRAM APP MSP:%#X\r\n", *(vu32*)(0X20001000));
                // LCD_ShowString(60,210,200,16,16,"Illegal SRAM APP!");	   
            }
            clearflag=7;//标志更新了显示,并且设置7*300ms后清除显示	 
        }
    }   	   
}







