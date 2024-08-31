
本实验为FLASH APP代码，
起始地址为0x08008000，不能直接下载运行(STM32默认从0x08000000开始运行),必须通过IAP Bootloader V1.0程序进行IAP更新.

Bootloader:
0x08000000~0x08007FFF  
size:0x8000(32KB)

App 地址空间:
0x08008000~0x080C0000 
size:0x38000(256-32=224KB)

