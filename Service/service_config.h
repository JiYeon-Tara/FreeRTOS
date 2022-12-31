#ifndef __SERVICE_CONFIG_H__
#define __SERVICE_CONFIG_H__


// AT 命令测试
#define AT_CMD_ENABLE               1
#define AT_CMD_TEST_ENABLE          1

// memory management
// 不可以关机, 一关机就 dump, 开不了机的那种
#define MEMORY_MANAGE_ENABLE        1
#define MEMORY_MANAGE_TEST_ENABLE   1

// file system
#define FS_ENABLE                   1
#define YAFFS_ENABLE                1
#define FATFS_ENABLE                1
#define FS_TEST_ENABLE              1




#endif


