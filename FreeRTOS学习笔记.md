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



---

##### TODO:

**1.内存池的概念究竟是怎么回事？？？**

只能看到定义内存池，但是找不到在哪里声明，怎么对应的呢？我怎么知道使用的是哪一块内存池？？？

![img](C:\Users\qz\AppData\Local\Temp\企业微信截图_16486316719339.png)

![img](C:\Users\qz\AppData\Local\Temp\企业微信截图_16486317682699.png)

仅仅是分配了内存池（内部其实就是pvPortMalloc），没有指向它的指针，如果free的呢？？？

![img](C:\Users\qz\AppData\Local\Temp\企业微信截图_16486317275663.png)

![img](C:\Users\qz\AppData\Local\Temp\企业微信截图_16486520302319.png)

为什么可以这么用呀？怎么关联内存池和指针之间的关系呢？

![img](C:\Users\qz\AppData\Local\Temp\企业微信截图_16486524474482.png)

终于找到原因了：传递的就是指针，把数据指针塞到队列里，队列里的指针指向的是堆内存，不会被释放（必须用户自己释放）

![img](C:\Users\qz\AppData\Local\Temp\企业微信截图_16486526631924.png)

接收数据塞入队列并发送：

![img](C:\Users\qz\AppData\Local\Temp\企业微信截图_16486530668141.png)

![img](C:\Users\qz\AppData\Local\Temp\企业微信截图_16486531064861.png)



#####  2. 以驱动的方式实现

![img](C:\Users\qz\AppData\Local\Temp\企业微信截图_16486545883239.png)

---

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



#####  4.线程间通信举例

```C
//thread.h
typedef struct
{
  	uint16_t msg_id; //
  	uint16_t msg_len;
} thread_msg_head_t; //线程间通信的消息头

// 线程间通信的消息示例, 实际 buf 大小根据实际情况定义
typedef struct
{
  	thread_msg_head_t head;
  	uint8_t msg_buf[1024];
} thread_msg_t;
```

通过 msg_id 将队列中的消息进行区分：

```C
//thread_bt.c
#define THREAD_BT_MSG_BUF_SIZE 	(4)

//任务通知
enum
{
  	//THREAD_BT_M_MSG_NOTIFY_DATA_RECV = 0x00000001;	// 数据接收
}

//消息 id
typedef enum
{
  	THREAD_BT_MSG_ID_BT_RESET	= 1, 	// 芯片复位	
} THREAD_BT_MSG_ID_E; //(给 thread_bt 队列里塞数据需要这个东西)

// 消息
typedef struct
{
  	thread_msg_head_t head;
  	uint8_t msg_buf[THREAD_MSG_BUF_SIZE];
} thread_bt_msg_t;
```

之所以区分是为了不同的应用场景：

```C
// 线程入口函数
void thread_loop(void)
{
  	xTaskNotifyWait(0, 0xFFFFFFFF, &notifyValue, osWaitForever);
  	dog_resume(dog_h); // 启动看门狗
  	if(notifyValue & SYSTEM_TASK_NOTIFY_MSG_READY)
    {
      	_thread_service_msg(); // 正常消息处理的部分
    }
  	if(notifyValue & SYSTEM_TASK_NOTIFY_SYNC)
    {
      	_thread_exit();
      	while(1)
        {
          	osDelay(osWaitForever);
        }
    }
}

// 从队列中取数据
void _thread_service_msg()
{
  	uint32_t ret;
  	thread_bt_msg_t *p_msg = NULL; // 使用内存池中的内存, 所以不需要动态分配内存
  
  	while(1)
    {
      	ret = osMessageGet(g_thread_bt.h_message_queue, &p_msg, 0);
      	if((ret != osEventMessage) || (p_msg == NULL)) // ERROR
        {
          	break;
        }
      	
      	switch(p_msg->head.msg_id) // 通过这里来区分不同的应用场景
        {
          case THREAD_BT_MSG_ID_RESET:
          {
			  break;
          }
            
          default:
            	break;
        }
    }
  	osPoolFress(g_thread_bt.h_poolID, p_msg);
}
```



##### 5.log 的使用

方便快捷的打开不需要的log的方法

```C
#if 1
#define BT_LOG_D(...) LOG_D(##__VA_ARGS__)
#define BT_LOG_E(...) LOG_E(##__VA_ARGS__)
#else
#define BT_LOG_D(...)
#define BT_LOG_E(...)
#endif
```



##### 6.串口中断接收这部分 ？？？

![img](C:\Users\qz\AppData\Local\Temp\企业微信截图_16486283476283.png)



##### 7.枚举类型

```C
typedef enum
{
  	BT_SPP_HANDLER_UNKNOWN = 0, 	// unknown
  	BT_SPP_HANDLER_TEST_APP,		// test APP
  	BT_SPP_HANDLER_APP,				// formal APP
  	BT_SPP_HANDLER_TV,				// TV
} SPP_HANDLER_E;	// 枚举类型 enumerate, 变量结尾 XX_YY_ZZ_E
```



