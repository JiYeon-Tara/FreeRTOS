##### DFU(Device Firmware Update)



##### 1.OTA 升级

Questions:

(1)OTA 升级流程中，如何处理设备原来保存的内容呢，这些内容是保存在哪里的呢？肯定不是 flash 中存储应用程序的分区是吗？



安卓软件升级?

安卓中的固件升级？

DFU(Device Firmware Update):固件升级, 可以有多种方式 2G/3G/4G/5G/wifi/NFC/zigbee 等通过无线升级的叫做 OTA, 也可以通过 USART/SPI/I2C 等硬件连接方式进行固件升级。

nordic nrf52832 OTA : https://www.cnblogs.com/iini/p/9314246.html

![image-20211212200049297](C:\Users\qz\AppData\Roaming\Typora\typora-user-images\image-20211212200049297.png)

![image-20211212200841074](C:\Users\qz\AppData\Roaming\Typora\typora-user-images\image-20211212200841074.png)

**BLE，UART和USB只是通信方式不一样，他们遵守的DFU流程是一模一样的，这里会以BLE通信接口为例，详细阐述DFU过程，UART和USB与之类似，就不再赘述。**



step1:制作 OTA 升级包;



##### 2. STM32F103 OTA 升级以及 bootloader





##### 3.RealTek 固件升级

(1) RTL 进入 HCI 模式；

(2) RTK 进入接收升级包，有一些参数协商与验证过程；

(3) Ap uart 发送数据包到 RTK，RTK 开始接收升级包;

(4) RTK 自行升级;



##### 4.bootloader

(1)如果 falsh 空间足够, 是可以存放很多不同的 app.bin 的;

(2) **STM32 的 APP 程序可以放到 FLASH 里面运行, 也可以放到 SRAM 里面运行**，具体参考不同的例子；

- 没有 bootloader之前，正常情况下 STM32 运行流程:

（1）0x08000000, flash 物理地址，从 0x08000004 （中断向量表 NVIC 起始地址）复位中断向量，包括 不可屏蔽中断 NMI，硬件异常 HardFault 等等；

（2）从复位中断程序入口（ResetHandler）开始执行，地址 0x08000004 + n，n 为中断向量表中中断的个数；

（3）从 ResetHandler （汇编实现）跳转到 main 函数，地址 0x08000000 + N，N 为中断向量表偏移量 + 所有中断服务程序入栈后的偏移；

（4）当有中断发生时，先到达 中断向量表 NVIC；

（5）根据中断向量表，执行中断相应的中断服务程序，中断向量表地址:0x08000004，**当有中断来临，STM32 内部的硬件会将 PC 指针指向 "中断向量表" 处，并根据中断源，取出对应的中断服务程序**；



- 添加 bootloader 之后，程序运行流程

（1）从 0x08000004 加在中断向量表；

（2）到达 0x08000004 + N 处执行 main 函数；

（3）**IAP过程（将新的 APP 代码吸入到 STM32 flash 中），跳转到新的中断向量表，新的 app 程序**;

（4）从 0x08000004 + N + M 处加载新的中断向量表，**中断向量表的位置不一样，但是中断服务函数一样**；

（5）跳转到新的程序的 main 函数；

（6）当有中断发生时，PC 指针指向旧的中断向量表，但是执行新的 app 的中断服务程序；



**IAP 程序的要求：**

1） 新程序必须在 IAP 程序之后的某个偏移量为 x 的地址开始；

2） 必须将新程序的中断向量表相应的移动，移动的偏移量为 x；



两种不同 APP 的升级方法：

- flash APP

片上 flash - 存放代码区

address:0x08000000 - 0x08040000

size:0x40000(256K)

增加 bootloader 以后, 这里 bootloader 的大小不是固定的, 可以根据 Bootloader 大小进行不同设置;

address:0x08008000 - 0x08040000, 偏移了 0x8000(32K)

size:256K - 32K = 224K

**注意：这里bootloader 大小 32K 不是固定的，可以根据需要进行调整，留有一定的余量**

SRAM 地址仍然是整块 SRAM 空间

address: 0x20000000 - 0x2000C000

size:0xC000 = 48 KByte



- SRAM APP

bootloader:0x20000000 - 0x20001000, 最开始的 4K bootloader;

APP:0x20001000 - 0x2000B000, 中间的 40K 存放 APP 程序;

0x2000B000 - 0x2000C000, 最后的 4K 用做 APP 程序的内存



##### 5. bootloader 部分的代码越小越好

一般会在 bootloader 中断点亮 LCD 屏幕，然后跳转到 APP 从而实现开机画面的效果。













