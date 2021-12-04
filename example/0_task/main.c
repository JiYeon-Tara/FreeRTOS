#include "main.h"

/************************* FreeRTOS 框架搭建好以后，进行简单测试 *******************/
int main(void)
{
	//设置系统中断优先级分组4，	
	//0-15 总共16级优先级，没有抢占优先级	 	 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init();	    				//延时函数初始化	  
	uart_init(115200);					//初始化串口
	LED_Init();		  					//初始化LED
    TIM3_Int_Init(10000-1,7200-1);		
	TIM5_Int_Init(10000-1,7200-1);		
    
	//创建开始任务
	xTaskCreate((TaskFunction_t )start_task,            //任务入口函数
				(const char*    )"start_task",          //任务名称
				(uint16_t       )START_STK_SIZE,        //任务堆栈大小
				(void*          )NULL,                  //传递给任务函数的参数
				(UBaseType_t    )START_TASK_PRIO,       //任务优先级
				(TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
	vTaskStartScheduler();          //开启任务调度
				
	return 0;		//正常来说执行不到这里
}


//开始任务入口函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区,临界段代码保护
    // //创建LED0任务
    // xTaskCreate((TaskFunction_t )led0_task,     	
    //             (const char*    )"led0_task",   	
    //             (uint16_t       )LED0_STK_SIZE, 
    //             (void*          )NULL,				
    //             (UBaseType_t    )LED0_TASK_PRIO,	
    //             (TaskHandle_t*  )&LED0Task_Handler);   
    // //创建LED1任务
    // xTaskCreate((TaskFunction_t )led1_task,     
    //             (const char*    )"led1_task",   
    //             (uint16_t       )LED1_STK_SIZE, 
    //             (void*          )NULL,
    //             (UBaseType_t    )LED1_TASK_PRIO,
    //             (TaskHandle_t*  )&LED1Task_Handler); 

    xTaskCreate((TaskFunction_t )interrupt_task,     
                (const char*    )"interrupt_task",   
                (uint16_t       )INTERRUPT_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )INTERRUPT_TASK_PRIO,
                (TaskHandle_t*  )&INTERRUPTTask_Handler); 
    //
    // 当任务退出 while(1) 时, 一定要删除任务
    //      
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
	
	return;
}




