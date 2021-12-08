#include "main.h"

/************************* FreeRTOS 框架搭建好以后，进行简单测试 *******************/
int main(void)
{
    hardware_init();
	software_init();

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






