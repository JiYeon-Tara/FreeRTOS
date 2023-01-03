#include "thread_key.h"
#include "thread_manager.h"


/********************
 * MACRO
 ********************/


/********************
 * FUNCTION
 ********************/
void key_task(void *pvParameters);
static void key_task_exit(void *param);


/********************
 * GLOBAL VAR
 ********************/
thread_cb_t key_thread = {
	.thread_init = key_task,
	.thread_deinit = key_task_exit,
};

xQueueHandle Key_Queue;          //��Ϣ���о��
xSemaphoreHandle key_sema;      //�������ź���
extern EventGroupHandle_t manager_event_group;  




/**
 * @brief hardware_init
 * 
 */
static void hardware_init()
{
    KEY_Init();

    return;
}

/**
 * @brief software_init
 * 
 */
static void software_init()
{

}

/**
 * @brief resource_init
 * 
 */
static void resource_init()
{
    //Queue
    Key_Queue = xQueueCreate(KEY_Q_SIZE, sizeof(uint8_t));  //��ʼ����Ϣ����
    //Semaphore
    key_sema = xSemaphoreCreateCounting(255, 0);
    // event group
    manager_event_group = xEventGroupCreate();

}

/**
 * @brief key_task
 * 
 * @param pvParameters 
 */
void key_task(void *pvParameters)
{
    BaseType_t ret;
    uint8_t key = 0;
    uint8_t cntSemVal = 0;

    taskENTER_CRITICAL();
    hardware_init();
    software_init();
    resource_init();
    printf("thread key running...\r\n");
	taskEXIT_CRITICAL();

    while(1)
    {
        //printf("Key thread running...\r\n");
        //���в���ϵͳ�ĵط�ʹ�� ��ѯ����������
        key = KEY_Scan(0);

        //����
        //printf("thread_key = %d\r\n", key);
        // if(Key_Queue!=NULL && key!=0)
        // {
        //     xQueueSend(Key_Queue, &key, 0);   //���͵���Ϣ����
        // }
        switch(key)
        {
            /********************* ���� WKUP ���� *********************************/
            //���°���һֱ��������, ��������Ҳ��Ч��
            case WKUP_PRES:
                //vTaskSuspend(Task1Task_Handler); //�������� 1 (2)
                //printf("�������� 1 ������!\r\n");
                break;
            case KEY0_PRES:
                //vTaskResume(Task1Task_Handler); //�ָ����� 1 (3)
                //printf("�ָ����� 1 ������!\r\n");
                //
                // �ͷ��ź���
                //
                ret = xSemaphoreGive(key_sema);
                if(ret == pdFALSE)
                {
                    printf("give semaphore failed.\r\n");
                }
                cntSemVal = uxSemaphoreGetCount(key_sema); //��ȡ�������ź�����ֵ
                printf("key thread give semaphore, sema val:%d\r\n", cntSemVal);
                break;
            case KEY1_PRES:
                //֪ͨ�����̳߳�ʼ�� -> ͨ��ʹ������֪ͨʵ��
                //���ñ�־λ
                xEventGroupSetBits(manager_event_group, TASK_SYNC);
                //vTaskSuspend(Task2Task_Handler);//�������� 2 (4)
                //printf("�������� 2 ������!\r\n");
                break;
            default:
                //printf("%s No keyboard input.\r\n", __func__);
                break;
        }

        // vTaskDelay(10); //��ʱ 10ms
    }
}

static void key_task_exit(void *param)
{
	
}
