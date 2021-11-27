The BLE project to learn BT/BLE protocol stack.
Hardware: Nordic Nrf52832
software: not open source BLE protocol stack, s132.(can use other firmware provided by Nordic )

BLE工程结构（0目录分配）:

**NOTE:分层设置，应用层、协议层和硬件设备分开编译，这种方式对后面编写私有任务非常有利**

**None:** arm_startup_nrf52.s(芯片启动文件) 和  system_nrf52.c(芯片系统文件);

**Application:**主函数main.c 以及 sdk_config.h(系统配置文件, 各种外设以及驱动都通过这个文件进行配置);

**BoardDefinition:**板载定义，与按键以及LED功能有关的设置;

**nRF_BLE:**保存与蓝牙服务有关的部分，如：广播的设置，连接参数的配置，GATT配置代码，peer 设备管理的代码以及安全参数设置等;

**nRF_Drivers：**外设驱动有关的代码，nrfx_xxx 表示新驱动，如:usart, clock, gpiote等;

**nRF_Liraries:**一些现成的与NRF芯片处理有关的函数，app_xxx表示和应用有关的函数库;

**nRF_Log 和 nRF_Segger_RTT:**提供一个log输出接口， log 输出有两种方式:

（1）uart串口，在log驱动文件夹中进行配置，ulog的使用; 

（2）在串口被占用的时候使用j-link的RTT进行打印;

**nRF_SoftDevice:**与协议栈有关的参数设定，没有开源，只留下了接口;

**不建议从头开始创建工程，耗费时间**



##### 1.BLE base

芯片内部内存分配情况:

分成两个部分，SoftDevice 152KB 和 Application 300KB ，如果要添加BOOT的话，则会分成三个空间（OTA升级中要用到，实际就是更新SoftDevice固件）。

![image-20211123223637469](C:\Users\qz\AppData\Roaming\Typora\typora-user-images\image-20211123223637469.png)


##### 2.advanced 
