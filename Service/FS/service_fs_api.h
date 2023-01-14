#ifndef __FS_API_H__
#define __FS_API_H__

#include <stdbool.h>
#include <stdint.h>

#include "exfuns.h"
#include "fattester.h" // 对 FATFS 接口的封装, 一个简单地测试代码，不足以作为中间层, 真正要用还是要自己直接封装 FATFS 的接口
#include "ff.h"



/********************
 * MACRO
 ********************/
// #define FS_FILE_SYSTEM_NAME             "/nand"         // 挂载文件系统目录
#define FS_FILE_SYSTEM_NAME             "0:"         // 挂载文件系统目录

// file operation
#define	FA_READ				0x01
#define	FA_OPEN_EXISTING	0x00
#if !_FS_READONLY
#define	FA_WRITE			0x02
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define FA__WRITTEN			0x20
#define FA__DIRTY			0x40
#endif

// file operation 

/********************
 * TYPEDEF
 ********************/
typedef struct {
    uint8_t *data;
    uint32_t len;
    void (*pProtect)(void);
    void (*pUnprotect)(void);
} buffer_area_t;

typedef void (*fs_dump_cb_t)(const char *format, ...);

void file_system_ctx_init(void);
void file_system_format(bool start);
void file_system_shutdown(void);
uint32_t path_hash(const char *path);
int open(const char *path, int mode);
int close(int handle);
int read(int handle, void *buff, int len);
int read_and_sync(int handle, uint8_t *buff, int len);
int read_seek(int handle, int64_t offset, void *buff, int len);
int open_read_seek(const char *path, int mode, int64_t offset, void *buff, int len);
int write(int handle, const void *buff, int len);
int write_and_sync(int handle, const uint8_t *buff, int len);
int open_write(const char *name, uint8_t *buff, uint32_t len);
void close_opened_file(const char *path);
int sync(int handle);
int unlink(const char *path);
int lseek(int handle, int64_t offset, uint8_t whence);
int64_t fsize(int handle);
int fileeof(int handle);
int ftruncate(int handle);
int fgetfree(char *pDrv);
uint32_t fgettotal(char *pDrv);
int fstat(const char *path, void *fstat);
int rename(const char *pathOld, const char *pathNew);
int copy_file(const char *pathOld, const char *pathNew, uint8_t *inCopyBuff, uint32_t inCopyBuffMaxSize);
int mkdir(const char *path);
int mkdir_p(const char *path);
int opendir(const char *path);
int closedir(int handle);
int readdir(int handle, void *info);
int recursively_delete(const char *path, int rmFileOnly);
int rmdir(const char *path, int rmFileOnly);
int find_dir(const char *path);
void close_opened_dir(const char *path);
int cut_folder(const char *old_path, const char *new_path, buffer_area_t *temp_data);
int file_tree(const char *p_root_path, char name_list[][128], uint32_t list_len);
void fs_print_opened_files(fs_dump_cb_t cb);
void dump_directory_tree(const char *path);


#endif

