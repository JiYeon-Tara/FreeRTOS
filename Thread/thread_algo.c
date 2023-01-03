#include "thread_algo.h"
#include "sys.h"

/********************
 * MACRO
 ********************/


/********************
 * FUNCTION
 ********************/
static void thread_algo_enery(void *pvParameters);
static void algo_task_exit(void *param);


/********************
 * GLOBAL VAR
 ********************/
thread_cb_t algo_thread = {
	.thread_init = thread_algo_enery,
	.thread_deinit = algo_task_exit,
};

static StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE]; //
static StaticTask_t IdleTaskTCB; //

static StackType_t TimerTaskStack[configTIMER_TASK_STACK_DEPTH]; //
static StaticTask_t TimerTaskTCB; //

StackType_t ThreadAlgoStack[THREAD_ALGO_STK_SIZE]; //
StaticTask_t ThreadAlgoTCB;             //
TaskHandle_t ThreadAlgo_Handler;        //


//���ʹ�þ�̬��
// �� �� �� �� Ҫ �� �� ʵ �� �� �� �� �� vApplicationGetIdleTaskMemory() ��
// vApplicationGetTimerTaskMemory()��ͨ������������������������Ͷ�ʱ����������������
// ջ��������ƿ�����ڴ棬���������������� mainc.c �ж��壬

/**
 * @brief ��������������ջ��������ƿ���ڴ�Ӧ�����û����ṩ
 * 
 * @param ppxIdleTaskTCBBuffer ������ƿ��ڴ�
 * @param ppxIdleTaskStackBuffer �����ջ�ڴ�
 * @param pulIdleTaskStackSize �����ջ��С
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
 * @brief ��ȡ��ʱ�������������ƿ��ڴ��
 *         �� �� �� �� �� �� �� �� �� �� ʱ �� �� �� �� �� �� API ���������
*          vApplicationGetIdleTaskMemory()�� vApplicationGetTimerTaskMemory()����ȡ��Щ�ڴ档
 * @param ppxTimerTaskTCBBuffer ������ƿ��ڴ�
 * @param ppxTimerTaskStackBuffer �����ջ�ڴ�
 * @param pulTimerTaskStackSize �����ջ��С
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
 * @brief �߳���ں���
 * 
 * @param pvParameters 
 */
static void thread_algo_enery(void *pvParameters)
{
    taskENTER_CRITICAL();
    printf("thread algorithm running...\r\n");
	taskEXIT_CRITICAL();

    while(1)
    {
        //printf("Thread_algo running...\r\n");
        vTaskDelay(2000);
    };
}

static void algo_task_exit(void *param)
{
	
}

