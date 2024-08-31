实验器材:
	MiniSTM32F103开发板V3版本
	
实验目的:
	学习STM32的IAP功能,掌握IAP的使用.
	
硬件资源:
	1,DS0(连接在PA8) 
	2,串口1(波特率:25600,PA9/PA10连接在板载USB转串口芯片CH340上面)
	3,ALIENTEK 2.8/3.5/4.3/7寸TFTLCD模块(通过GPIO驱动,连接关系见lcd.h) 
	4,按键KEY0(PC5)/KEY1(PA15)/KEY_UP(PA0,也称之为WK_UP)  
		

IAP 例程实现功能：
开机的时候先显示提示信息，然后等待串口输入接收 APP 程序（无校验，一次性接收）在串口接收到 APP 程序之后，即可执行 IAP。
a.如果是 SRAM APP，通过按下 KEY0 即可执行这个收到的 SRAM APP 程序。
b.如果是 FLASH APP，则需要先按下 WK_UP 按键，将串口接收到的 APP 程序存放到 STM32 的 FLASH，之后再按KEY1 既可以执行这个 FLASH APP 程序。


5,本实验串口1波特率需要设置为：25600！！！
6,本实验下载成功后,需用串口调试助手发送SRAM APP/FLASH APP等APP代码(.bin文件)验证IAP功能. 
7,SRAM APP代码的起始地址必须是：0X20001000，FLASH APP代码的起始地址必须是：0X08008000.
8,大家在创建APP代码的时候，切记要设置中断向量偏移量！！
	  

					
					
					
					
					
					
					
					
					