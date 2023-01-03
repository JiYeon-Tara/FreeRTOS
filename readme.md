问题们:

(1)堆栈分配在哪里, 都是在 RAM 吗？ 有没有可能在 flash ， 应该不会， flash 相当于磁盘，
(2)后面可以添加一个cmsis.h, cmsis.c 提供接口, 将 FreeRTOS 的接口进行封装, 这样切换操作系统时，只需要修改这个文件即可, 增强代码的可移植性;

**好好学学"彭志伟"写的代码，alarm 也是使用链表进行管理的，db_alarm_clock.c**

音乐播放也是使用链表进行管理，

(5) 对 GUI 进行管理，每一个小方块代表一个线程，显示每个线程收到消息的情况以及线程有关的信息；

| name           | info            |
| -------------- | --------------- |
| Thread manager | msg:16 info:abc |
| Thread monitor | msg:17 info:def |
| Thread key     | msg:5 info:ddd  |
| Thread LED     | msg:8 info:www  |
| Thread ...     | msg:1 info:zzz  |



##### FreeRTOS

##### 1.Task

```C
BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                        const char * const pcName,
                        const uint16_t usStackDepth,
                        void * const pvParameters,
                        UBaseType_t uxPriority,
                        TaskHandle_t * const pxCreatedTask );
```

?	任务需要 RAM 来保存与任务有关的状态信息(任务控制块)，任务也需要一定的 RAM 来作为任务堆栈。如果使用函数 xTaskCreate()来创建任务的话，那么这些**所需的 RAM 就会自动的从 FreeRTOS 的堆中分配**，**因此必须提供内存管理文件，默认我们使用heap_4.c 这个内存管理文件，而且宏 configSUPPORT_DYNAMIC_ALLOCATION 必须为 1**。如果使用函数 xTaskCreateStatic()创建的话这些 RAM 就需要用户来提供了。

?	新创建的任务默认就是就绪态的，如果当前没有比它更高优先级的任务运行那么此任务就会立即进入运行态开始运行，不管在任务调度器启动前还是启动后，都可以创建任务。

###### 1.1 队列

用于在任务与任务，任务与中断之间传送数据，将数据拷贝到队列中，其它任务取出，效率比较低;

UCOS 操作系统就传递数据的引用（指针），但是必须要保证数据存在;

**直接往队列里放数据的指针也可以实现传递数据引用的目的，同样的，要保证数据存在，这样当我要发送的消息数据太大的时候就可以直接发送消息缓冲区的地址指针，**

eg:

比如在网络应用环境中，网络的数据量往往都很大的，采用数据拷贝的话就不现实。

创建队列:

```C
QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize);
```

此函数本质上是一个宏，用来动态创建队列，此宏最终调用的是函数 xQueueGenericCreate()，

###### 1.2 信号量

信号量可以用于控制对共享资源的访问，也可以用于线程同步;

二值信号量:用于任务与任务 以及 任务与中断之间的同步;

计数型信号量

互斥信号量

##### 问题:串口中释放信号量，出现如下错误：

**还没有解决**

错误:Error:..\..\FreeRTOS\portable\RVDS\ARM_CM3\port.c,714

![image-20220116202846592](C:\Users\qz\AppData\Roaming\Typora\typora-user-images\image-20220116202846592.png)



##### 互斥信号量:

（1）互斥信号量有优先级继承的机制，所以只能用在任务中，不可以用在中断服务函数中；

（2）中断服务函数中不可以因为要等待互斥信号量而进入阻塞态（**任务可以阻塞，但是中断绝对不可以**）;



##### 软件定时器

（1）软件定时器的回调函数中绝对不可以使用延时 或者会导致阻塞的函数（如等待入队，信号量等）



##### 事件标志组

信号量只能同步单个任务与事件，某个任务需要与多个任务或者事件进行同步时——事件标志组；



##### 2. 使用外部设备 DEVICE_EXPORT ( Linux 驱动中会见到这种写法, FreeRTOS 驱动模仿 Linux)

**DEVICE_EXPORT**

service 层代码分上层API 和 下层 API， 下层API供驱动底层调用，上层API 供应用层调用，是不是Linux的service也是这样实现的？（大概率是，毕竟以前都是做手机的）

https://zhuanlan.zhihu.com/p/356482765



```C
// dev_bt_spp.c 中
bt_spp_drv_io_t user_bt_spp_ops = 
{
  	// .init
  	.spp_send = user_spp_send,
 	.spp_recv = user_spp_recv,
	// .deinit
};
DEVICE_EXPORT(dev_bt_spp_link, &user_bt_spp_ops);
```

service 库中:

device_bt_spp.h

```C
// 操作接口
typedef struct
{
  	bool (*spp_send)(BT_SPP_HANDLER_E handler, uint8_t channel, uint8_t *p_data, uint 16_t len);
  	// 定义其它操作接口, init, deinit, send, recv, 典型的 Linux 驱动写法
}

// abstract device type
typedef struct
{
  	const char *name;	//device name
	const bt_spp_drv_io_t *ops;	// device IO of BT SPP
} bt_spp_drv_type_t;  // 抽象设备类型
```

dev_bt_spp.c

```C
bt_spp_drv_type_t g_bt_spp_deviec;
// dev_bt_spp.c
void dev_bt_spp_link(const void *pdriver)
{
  	g_bt_spp_device.name = "Dev-bt-spp";
  	g_bt_spp_device.ops  = (bt_spp_drv_io_t*)pdriver;
}
```

用户接口方面：不直接调用底层接口，通过封装好的 user_xx_yy 接口

user_bt_spp.c

```C
// 发送函数（供上层用户调用的接口）
void user_spp_send(BT_SPP_HANDLER_E handler, uint8_t channel, uint8_t *p_data, uint16_t len)
{
  	// 根据不同的 Hander, channel 封装不同包头的数据 发送到 bt_send_thread 的队列中 
}

bt_spp_drv_io_t user_bt_spp_ops = 
{
  	.spp_send = user_spp_send,
};
DEVICE_EXPORT(dev_bt_spp_link, &user_bt_spp_ops); // 关键操作
```

**DEVICE_EXPORT 内部是如何实现的?**

spp_def.h

```C
// function define of abstract device
typedef void (*dev_ops_link)(const void *ops_ptr);

// 设备描述
typedef struct _dev_desc
{
  	const char 			*func_name;
  	const void 			*ops_ptr;
  	const dev_ops_link	func;
} dev_desc_t;
#define _DEVICE_EXPORT(fn, ops_ptr, level) \
	const char _dev_init_##fn##_name[] = #fn; \ // ## 字符串拼接, # 将其它值变成字符串
	const dev_desc_t _dev_desc_##fn __attribute__((used, section(".device_init.$"level)))= \
							{_dev_init_##fn##_name, ops_ptr, &fn} // 看不懂的地方

// (function name, operation_pointer)
#define DEVICE_EXPORT(fn, ops_ptr) _DEVICE_EXPORT(fn, ops_ptr, "f") // C 语言中使用可变参数的方式
```



##### 5.不同的协议支持不同的应用场景——如何通过一条底层链路（蓝牙链路）支持多种不同的应用场景（向大佬万杰多学习）

```C
void proto_data_recv(HANDLER_E handler, uint8_t channel, uint8_t *p_data, uint 32_t len);
```







##### 6. nuttx 操作系统学习



