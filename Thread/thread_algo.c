#include "thread_algo.h"
#include "sys.h"

/**************************** global varible ******************************/
static StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE]; //空闲任务任务堆栈
static StaticTask_t IdleTaskTCB; //空闲任务控制块

static StackType_t TimerTaskStack[configTIMER_TASK_STACK_DEPTH]; //定时器服务任务堆栈
static StaticTask_t TimerTaskTCB; //定时器服务任务控制块

/**************************** macro definition ******************************/

/**************************** macro definition ******************************/

//创建thread algorithm任务有关
StackType_t ThreadAlgoStack[THREAD_ALGO_STK_SIZE]; //任务堆栈
StaticTask_t ThreadAlgoTCB;             //任务控制块
TaskHandle_t ThreadAlgo_Handler;        //任务句柄


//如果使用静态方
// 法 的 话 需 要 用 户 实 现 两 个 函 数 vApplicationGetIdleTaskMemory() 和
// vApplicationGetTimerTaskMemory()。通过这两个函数来给空闲任务和定时器服务任务的任务堆
// 栈和任务控制块分配内存，这两个函数我们在 mainc.c 中定义，

/**
 * @brief 空闲任务的任务堆栈和任务控制块的内存应该由用户来提供
 * 
 * @param ppxIdleTaskTCBBuffer 任务控制块内存
 * @param ppxIdleTaskStackBuffer 任务堆栈内存
 * @param pulIdleTaskStackSize 任务堆栈大小
 */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, 
                                    StackType_t **ppxIdleTaskStackBuffer, 
                                    uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &IdleTaskTCB;
    *ppxIdleTaskStackBuffer = IdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/**
 * @brief 获取定时器任务的任务控制块内存等
 *         最 后 创 建 空 闲 任 务 和 定 时 器 服 务 任 务 的 API 函数会调用
*          vApplicationGetIdleTaskMemory()和 vApplicationGetTimerTaskMemory()来获取这些内存。
 * @param ppxTimerTaskTCBBuffer 任务控制块内存
 * @param ppxTimerTaskStackBuffer 任务堆栈内存
 * @param pulTimerTaskStackSize 任务堆栈大小
 * @retval None
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
                                    StackType_t **ppxTimerTaskStackBuffer, 
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &TimerTaskTCB;
    *ppxTimerTaskStackBuffer = TimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/**
 * @brief 线程入口函数
 * 
 * @param pvParameters 
 */
void thread_algo_enery(void *pvParameters) //任务函数
{

    while(1)
    {
        //printf("Thread_algo running...\r\n");
        vTaskDelay(2000);
    }
}

