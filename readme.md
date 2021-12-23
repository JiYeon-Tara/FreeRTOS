问题们:

(1)堆栈分配在哪里, 都是在 RAM 吗？ 有没有可能在 flash ， 应该不会， flash 相当于磁盘，
(2)后面可以添加一个cmsis.h, cmsis.c 提供接口, 将 FreeRTOS 的接口进行封装, 这样切换操作系统时，只需要修改这个文件即可, 增强代码的可移植性;



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

