#ifndef __SERVICE_CONFIG_H__
#define __SERVICE_CONFIG_H__


// AT 命令测试
#define AT_CMD_ENABLE               1
#define AT_CMD_TEST_ENABLE          1

// memory management
#define MEMORY_MANAGE_ENABLE        1
#define MEMORY_MANAGE_TEST_ENABLE   1

// file system
#define FS_ENABLE                   1
#define YAFFS_ENABLE                0
#define YAFFS_TEST_ENABLE           0
#define FATFS_ENABLE                1
#define FATFS_TEST_ENABLE           1
#define FS_API_ENABLE               0
#define FS_API_TEST_ENABLE          0
#define FS_WORD_SAVE_AT_EXT_FLASH   1 // 字库通过 C2B 工具转换后保存到外部 flash

// asm test
#define SERVICE_ASM_TEST            1

#endif


