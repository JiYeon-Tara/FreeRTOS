/**
 * @file main.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "main.h"

/************************* FreeRTOS ��ܴ���Ժ󣬽��м򵥲��� *******************/
int main(void)
{
    hardware_init();
	software_init();

	xTaskCreate((TaskFunction_t )start_task,            //
				(const char*    )"start_task",          //
				(uint16_t       )START_STK_SIZE,        //
				(void*          )NULL,                  //
				(UBaseType_t    )START_TASK_PRIO,       //
				(TaskHandle_t*  )&StartTask_Handler);   //              
	vTaskStartScheduler();          //
    		
	return 0;
}






