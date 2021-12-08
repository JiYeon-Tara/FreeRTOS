#include "main.h"
#include "timer.h"
#include "my_timer.h"

#include "led0_thread.h"
#include "led1_thread.h"
#include "interrupt_thread.h"
#include "thread_ui.h"
#include "thread_algo.h"
#include "thread_key.h"
#include "thread_dual_comm.h"
/**************************** global varible ******************************/
TaskHandle_t StartTask_Handler;     //任务句柄

/**************************** macro definition ******************************/

/**************************** macro definition ******************************/


/**
 * @brief hardware_init
 * 
 */
void hardware_init()
{
    //设置系统中断优先级分组4，	
	//0-15 总共16级优先级，没有抢占优先级	 	 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();	    				//延时函数初始化	  
	uart_init(115200);					//初始化串口
	//LED_Init();		  					//初始化LED

    return;
}

/**
 * @brief software_init
 * 
 */
void software_init()
{
    // TIM3_Int_Init(10000-1,7200-1);		
	// TIM5_Int_Init(10000-1,7200-1);	

    return;
}

/**
 * @brief  thread manager thread entry function
 * 
 * @param pvParameters 
 */
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区,临界段代码保护

    //
    //创建LED0任务
    //
    // xTaskCreate((TaskFunction_t )led0_task,     	
    //             (const char*    )"led0_task",   	
    //             (uint16_t       )LED0_STK_SIZE, 
    //             (void*          )NULL,				
    //             (UBaseType_t    )LED0_TASK_PRIO,	
    //             (TaskHandle_t*  )&LED0Task_Handler);  

    //
    //创建LED1任务
    //
    // xTaskCreate((TaskFunction_t )led1_task,     
    //             (const char*    )"led1_task",   
    //             (uint16_t       )LED1_STK_SIZE, 
    //             (void*          )NULL,
    //             (UBaseType_t    )LED1_TASK_PRIO,
    //             (TaskHandle_t*  )&LED1Task_Handler); 

    //
    // key task
    //
    xTaskCreate((TaskFunction_t )key_task,     
                (const char*    )"key_task",   
                (uint16_t       )KEY_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )KEY_TASK_PRIO,
                (TaskHandle_t*  )&KeyTask_Handler); 

    //
    // interrupt task
    //
    xTaskCreate((TaskFunction_t )interrupt_task,     
                (const char*    )"interrupt_task",   
                (uint16_t       )INTERRUPT_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )INTERRUPT_TASK_PRIO,
                (TaskHandle_t*  )&INTERRUPTTask_Handler); 

    //
    // ui_task  
    //
    xTaskCreate((TaskFunction_t )thread_ui_entry,     
                (const char*    )"thread_ui",   
                (uint16_t       )THREAD_UI_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )THREAD_UI_PRIO,
                (TaskHandle_t*  )&Thread_UI_Handler); 
     
    // 当任务退出 while(1) 时, 一定要删除任务
    vTaskDelete(StartTask_Handler); //删除开始任务

    //
    // static create task
    // xTaskCreateStatic() 的返回值是任务句柄
    //
    ThreadAlgo_Handler = xTaskCreateStatic(	(TaskFunction_t )thread_algo_enery,
                                            (const char*    )"thread_algo",
                                            (uint16_t       )THREAD_ALGO_STK_SIZE,
                                            ( void *        )NULL,
                                            (UBaseType_t    )THREAD_ALGO_PRIO,
                                            (StackType_t*   )ThreadAlgoStack,
                                            (StaticTask_t*  )&ThreadAlgoTCB );
    taskEXIT_CRITICAL();            //退出临界区
	
	return;
}


