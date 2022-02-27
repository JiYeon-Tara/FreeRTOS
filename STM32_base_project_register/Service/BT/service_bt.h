/**
 * @file service_bt.h
 * @author your name (you@domain.com)
 * @brief (1)service layer process data from lower layer, eg:BSP layer
 *        (2)and send it to upper layer, eg: GUI, APP, proto...
 *        (3)统一上层接口, 方便移植
 *        (4)对所以设备以及数据使用 “面向对象” 的思想进行管理: DEVICE_EXPORT(func_ptr1, func_ptr2)
 *        (5)可以对 service 所有打包成一个静态库，只提供头文件
 * @version 0.1
 * @date 2022-02-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __SERVICE_BT_H__
#define __SERVICE_BT_H__
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


void service_bt_init(void);
void service_bt_send(uint8_t *p_data,uint32_t len);
void service_bt_receive(void);

#endif //end #ifndef __SERVICE_BT_H__

