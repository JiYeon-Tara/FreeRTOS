问题们:

(1)堆栈分配在哪里, 都是在 RAM 吗？ 有没有可能在 flash ， 应该不会， flash 相当于磁盘，



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
