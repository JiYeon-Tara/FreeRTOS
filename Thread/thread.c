#include "thread.h"
#include <stdlib.h>
#include <string.h>


/********************
 * MACRO
 ********************/

/********************
 * MACRO
 ********************/


/********************
 * function
 ********************/
/**
 * @brief 线程通信
 * 
 * @param[in] pThreadCb
 * @param[in] msg_id
 * @param[in] data 不需要对数据指针的生命周期进行管理, 内部会进行数据拷贝, 使用的是"目标线程"的内存池
 * @param[in] msg_len
*/
bool thread_msg_send(const thread_cb_t *pThreadCb, uint8_t msg_id, void *data, uint16_t msg_len)
{
    bool ret = false;
    if(!pThreadCb || !pThreadCb->taskHandle || !pThreadCb->queue){ // || pThreadCb->poolID
        return false;
    }

    // uint32_t itemSize = osPoolGetItemSize(pThreadCb->poolID);
    // if(itemSize < (msg_len + 4){
    //     return false;
    // }

    // thread_msg_t *pMsg = (thread_msg_t*)osPoolAlloc(pThreadCb->poolID);
    thread_msg_t *pMsg = (thread_msg_t*)malloc(sizeof(thread_msg_t));
    if(!pMsg){
        return false;
    }

    if(msg_len < 0){
        free(pMsg);
        return false;
    }

    pMsg->head.msg_id = msg_id;
    pMsg->head.msg_len = msg_len;
    memcpy((void*)pMsg->data, data, msg_len);

    // if(itemSize != (msg_len + 4){
    //     pMsg->data[msg_len] = 0;
    // }

    if(xQueueSend(pThreadCb->queue, (void*)pMsg, 5000) != pdPASS){
        printf("message send failed, msg_id:%d msg_len:%d\r\n", msg_id, msg_len);
        free(pMsg);
        return false;
    }

    if(xTaskNotify(pThreadCb->taskHandle, 1, eSetBits) != pdPASS){
        printf("notify send failed, msg_id:%d msg_len:%d\r\n", msg_id, msg_len);
        free(pMsg);
        return false;
    }

    return true;
}


