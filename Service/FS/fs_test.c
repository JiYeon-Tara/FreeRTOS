/**
 * @file fs_test.c
 * @author your name (you@domain.com)
 * @brief 文件系统测试
 * @version 0.1
 * @date 2023-01-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "fs_test.h"
#include "service_config.h"

#if FS_TEST_ENABLE
#include "service_fs_api.h"
#include "malloc.h"


/********************
 * MACRO
 ********************/
#define LOG_E(fmt, ...)     printf(fmt"\n", ##__VA_ARGS__)


/**
 * @brief 
 * 
 * @return int 0 - success
 *             -1 - failed
 */
int fs_test(void)
{
#define FS_TEST_ITEM_LEN        64
#define FS_TEST_TOTAL_LEN       (FS_TEST_ITEM_LEN * 1024) // 64K

    uint8_t tempBuf[FS_TEST_ITEM_LEN];
    int ret;
    int fd = 0;
    uint64_t i, j;

    // write test
    fd = open(FS_FILE_SYSTEM_NAME"/fs_test.bin", FA_OPEN_ALWAYS | FA_WRITE);
    if(fd < 0) {
        LOG_E("fs test error %d\r\n", fd);
        return fd;
    }

    for(i = 0; i < FS_TEST_TOTAL_LEN / FS_TEST_ITEM_LEN; ++i) {
        for(j = 0; j < FS_TEST_ITEM_LEN; ++j) {
            tempBuf[j] = i & 0xFF;
        }
        ret = write(fd, tempBuf, FS_TEST_ITEM_LEN);
        if(ret != FS_TEST_ITEM_LEN){
            LOG_E("fs test write failed %d\r\n", ret);
            close(fd);
            return ret;
        }
    }
    close(fd);

    fd = open(FS_FILE_SYSTEM_NAME"/fs_test.bin", FA_READ);
    if(fd < 0) {
        LOG_E("fs test error %d\r\n", fd);
        return fd;
    }

    for(i = 0; i < FS_TEST_TOTAL_LEN / FS_TEST_ITEM_LEN; ++i) {
        ret = read(fd, tempBuf, FS_TEST_ITEM_LEN);
        if(ret != FS_TEST_ITEM_LEN) {
            close(fd);
            LOG_E("fs test write failed %d\r\n", ret);
            return ret;
        }
        for(j = 0; j < FS_TEST_ITEM_LEN; ++j) {
            if(tempBuf[j] != (i %  0xFF)){
                LOG_E("file read test failed, %d != %d\r\n", tempBuf[j], i);
                close(fd);
                return -1;
            }
        }
    }

    ret = close(fd);
    if(ret != FR_OK) {
        LOG_E("fiel close failed:%d\r\n", ret);
        return ret;
    }
    printf("File Test OK\r\nSize:%dByte Test\r\n", FS_TEST_TOTAL_LEN);
    return 0;
}

/**
 * @brief 
 * 
 * @param is_read 
 * @param size 
 * @return int 
 */
int fs_speed_test(bool is_read)
{
    int fd;
    int ret;
    uint64_t startTick, endTick;
    uint8_t *pBuff = NULL; 
    uint8_t size = 1024; // 1KB

    if(is_read) {
        fd = open(FS_FILE_SYSTEM_NAME"/fs_test.bin", FA_READ);
    }
    else {
        fd = open(FS_FILE_SYSTEM_NAME"/fs_test.bin", FA_OPEN_ALWAYS | FA_WRITE);
    }

    pBuff = (uint8_t*)mymalloc(size);
    if(!pBuff) {
        LOG_E("memory allocate error\r\n");
        return -1;
    }

    startTick = 0;  // startTick = osKernelSysTick();
    if(is_read) {
        ret = read_seek(fd, 0, pBuff, size);
    }
    else{
        ret = write_and_sync(fd, pBuff, size);
    }
    endTick = 0; // endTick = osKernelSysTick();
    printf("fs speed test: size:%lu time %lu speed:%lf B/s", size, (endTick - startTick), (size) / (endTick - startTick));

    myfree(pBuff);
    close(fd);

    return;
}
#endif

