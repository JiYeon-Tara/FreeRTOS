/**
 * @file service_fs.c
 * @author your name (you@domain.com)
 * @brief 初始化应用程序需要的一些固有文件以及目录, 比如:log 目录, 资源目录...
 *        开机初始化的时候, 文件系统初始化完成就执行
 * @version 0.1
 * @date 2023-01-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "service_fs.h"
#include <stdbool.h>

/********************
 * MACRO
 ********************/
#define USER_ROOT_DIR   "0:/root"
#define USER_APP_DIR    "0:/app"
#define USER_OTA_DIR    "0:/ota"
#define RES_IMG_DIR     "0:/image"


/**
 * @brief user default directory init
 * 
 * @return bool 
 */
bool user_directory_init()
{
    bool ret = false;
    // service_fs_mkdir(USER_ROOT_DIR);
    // service_fs_mkdir(USER_APP_DIR);
    // service_fs_mkdir(USER_OTA_DIR);
    // service_fs_mkdir(RES_IMG_DIR);

    return ret;
}
