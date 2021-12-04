##### FreeRTOS学习笔记

**NOTE:**大胆的想法（家庭作业）：

(1)看到 LCD 可以做矩形框移动，我是不是可以做一个俄罗斯方块

(2)再进阶一下，使用 BSP8266实现使用Wifi 通信，通过互联网操作远程的 STM32（物联网），两个板子通过窗口或者其它方式通信，

(3)再扩展可以写一个手机APP, Web端页面， 软件， 手机小程序 ？？？服务器，后端也锻炼了， （租一个服务器）

(4)发朋友圈：不知道学这些有什么用，你可以通过手机 或者 电脑远程控制我的东西了，又想试试的可以扫一下二维码，

**大胆的想法2**

(1)通过 MPU6050 控制单轮 / 双轮(自行车) 平衡车，简单的 x, y 方向的 PID 调节???

(2)通过手柄等其它方式进行控制 ???

调试工具: Tracelyzer



##### 1.条件编译的重要性

```C
//通过条件编译的方式来控制编译后代码的大小
#if (INCLUDE_vTaskPrioritySet == 1)
	void vTaskPrioritySet( TaskHandle_t xTask, UBaseType_t uxNewPriority )
    {
        
    }
#endif
```

​	FreeRTOS 中的裁剪和配置就是这种用条件编译的方法来实现的，不止 FreeRTOS这么干，

很多的协议栈、RTOS 系统和 GUI 库等都是使用条件编译的方法来完成配置和裁剪的。条件编

译的好处就是节省空间，不需要的功能就不用编译，这样就可以根据实际需求来减少系统占用

的 ROM 和 RAM 大小，根据自己所使用的 MCU 来调整系统消耗，降低成本。



##### 2.回调函数

同步回调:立即执行

异步回调:在某个时间执行

回调函数的好处和作用： 解耦，不需要修改一个封装成库的源文件，即可以实现不同的功能；如果使用函数调用的话，则达不到这种效果。

```C
#include <stdio.h>
#include <softwarelib.h>		//software library

//callback function
int callback()
{
    return 0;
}

int main()
{
	library(callback);		//通过回调函数的方式
    
    return 0;
    
}

```



3.ARM 架构：



