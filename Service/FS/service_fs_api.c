/**
 * @file fs_api.c
 * @author your name (you@domain.com)
 * @brief file system API, abstract layer
 * @version 0.1 封装 FATFS 接口, 作用文件系统层, 对上层应用提供操作文件的接口, 例如:open(), write(), read(), close(), lseek()...
 * @date 2022-12-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "service_fs_api.h"
#include "malloc.h"



// FAFFS website:
// http://elm-chan.org/fsw/ff/00index_e.html
// 一般需要修改 ffconf.h 和 diskio.c 这两个文件


/********************
 * MACRO
 ********************/
#ifdef CONFIG_ENABLE_OS
#include "semphr.h"
#endif

#ifdef CONFIG_ENABLE_ULOG
#include "ulog.h"
#else
#define LOG_D(fmt, ...)     printf(fmt"\n", ##__VA_ARGS__)
#define LOG_E(fmt, ...)     printf(fmt"\n", ##__VA_ARGS__)
#define LOG_W(fmt, ...)     printf(fmt"\n", ##__VA_ARGS__)
#endif

#define ASSERT(x)           {if(!x) while(1);}

#define FILE_MAX                (3)       // 支持同时打开的文件书目, ffconf.h -> _FS_LOCK
#define DIR_MAX                 (3)       //
#define HANDLE_INCREASE_STEP    (180)

#define FILE_USING              (1)       // 文件被打开, 正在被操作
#define FILE_NOT_USING          (0)

#define DIR_USING               (1)       // 目录被打开, 正在被操作
#define DIR_NOT_USING           (0)

#define ROOT_DIR                "0:"      // 根目录名称
#define ROOT_DIR_NAND           "/nand"

// #define f_eof(fp) (((fp)->fptr == (fp)->fsize) ? 1 : 0)
// #define f_error(fp) ((fp)->err)
// #define f_tell(fp) ((fp)->fptr)
// #define f_size(fp) ((fp)->fsize)

/********************
 * TYPEDEF
 ********************/
typedef struct {
    uint8_t fileUsing;      // 文件正在使用中
    uint32_t fileTimeStamp; // 文件打开时间戳/os systick
    // uint32_t fileHandler;   // 文件系统文件句柄
    FIL *fileHandler;
    uint8_t fileName[_MAX_LFN]; // 文件系统文件名称
    uint32_t filePathHash;  // 文件名 hash 值
    int fileHandle;         // 记录当前被打开的文件句柄, 防止文件 API 操作错误对象
} fs_file_t;

typedef struct {
    uint8_t dirUsing;
    uint32_t dirTimeStamp;
    DIR *dirHandler;
    uint8_t dirName[_MAX_LFN];
    uint32_t dirPathHash;
    int dirHandle;           // 记录当前被打开的文件句柄, 防止文件 API 操作错误对象
} fs_dir_t;


/********************
 * GLOBAL VAR
 ********************/


/********************
 * STATIC VAR
 ********************/
static fs_file_t _g_file_opened[FILE_MAX];
static uint32_t _g_file_opened_num = 0;     // 打开文件总数
static volatile int _g_file_handle_base;    // 这里是为了防止打开错误的文件

static fs_dir_t _g_dir_opened[DIR_MAX];
static uint32_t _g_dir_opened_num = 0;      // 打开目录总数
static volatile int _g_dir_handle_base;     //


#if CONFIG_ENABLE_OS
static SemaphoreHandle_t _g_fs_mutex;
static StaticSemaphore_t _fs_mutex;
#else
#endif

/********************
 * STATIC FUNCTION
 ********************/


/********************
 * GLOBAL FUNCTION
 ********************/
extern int yaffsfs_GetError(void); // f_error(fp)

/**
 * @fun:
 * 
 * @ret:
*/
void file_system_ctx_init()
{
    uint8_t index = 0;

    memset(_g_file_opened, 0, sizeof(_g_file_opened));
    memset(_g_dir_opened, 0, sizeof(_g_dir_opened));

    // file object memory allocate
    for(index = 0; index < FILE_MAX; ++index) {
        _g_file_opened[index].fileHandler = (FIL*)mymalloc(sizeof(FIL) * 1);
        if(!_g_file_opened[index].fileHandler){
            LOG_E("file system file object memory malloc failed\r\n");
            ASSERT(0);
            return;
        }
    }

    // dir object
    for(index = 0; index < DIR_MAX; ++index) {
        _g_dir_opened[index].dirHandler = (DIR*)mymalloc(sizeof(DIR) * 1);
        if(!_g_dir_opened[index].dirHandler){
            LOG_E("file system dir object memory malloc failed\r\n");
            ASSERT(0);
            return;
        }
    }

#if CONFIG_ENABLE_OS
    if(!_g_fs_mutex){
        _g_fs_mutex = xSemaphoreCreateStatic(&_fs_mutex);
    }
#else
#endif

    return;
}

/**
 * @brief 上锁, 避免多线程操作文件系统出现异常
 * 
 */
static void FF_LOCK()
{
#if CONFIG_ENABLE_OS
    if(!osKernelRunning()){
        LOG_E("RTOS not running\r\n");
        return;
    }
    if(!_g_fs_mutex){
        LOG_E("null mutex\r\n");
        return;
    }
    xSemaphoreTake(_g_fs_mutex, portMAX_DELAY);
#else

#endif

    return;
}

static void FF_UNLOCK()
{
#if CONFIG_ENABLE_OS
    if(!osKernelRunning()){
        LOG_E("RTOS not running\r\n");
        return;
    }
    if(!_g_fs_mutex){
        LOG_E("null mutex\r\n");
        return;
    }
    xSemaphoreRelease(_g_fs_mutex, portMAX_DELAY);
#else

#endif
}

static uint32_t FF_GET_SYS_TICK(void)
{
#if CONFIG_ENABLE_OS
    return KernelSysTick()
#else
    return 0;
#endif
}

/**
 * @brief file system format
 * 
 * @param start 
 */
void file_system_format(bool start)
{
    uint8_t index;

    if(!start) { // 文件系统格式化结束, 允许操作
        FF_UNLOCK();
        return;
    }

    // 格式化开始, 阻塞所有文件操作
    FF_LOCK();

    // 关闭所有打开的文件
    for(index = 0; index < FILE_MAX; ++index) {
        if(_g_file_opened[index].fileUsing != FILE_USING) {
            continue;
        }
        f_close(_g_file_opened[index].fileHandler);
        memset(&_g_file_opened[index], 0, sizeof(fs_file_t));
        if(_g_file_opened_num) {
            --_g_file_opened_num;
        }
    }

    // close opened directory
    for(index = 0; index < DIR_MAX; ++index) {
        if(_g_dir_opened[index].dirUsing != DIR_USING) {
            continue;
        }
        f_closedir(_g_dir_opened[index].dirHandler);
        memset(&_g_dir_opened[index], 0, sizeof(fs_dir_t));
        if(_g_dir_opened_num) {
            --_g_dir_opened_num;
        }
    }

    // 格式化开始, 阻塞所有文件操作, 这里不调用 FF_UNLOCK
    return;
}

/**
 * @brief 文件系统关闭
 * 
 */
void file_system_shutdown()
{
    file_system_format(true);
    // fs_unmount(FS_FILE_SYSTEM_NAME);

    return;
}

void file_system_ff_lock()
{
    FF_LOCK();
}

void file_system_ff_unlock()
{
    FF_UNLOCK();
}

/**
 * @brief 计算文件名 hash 值, BDK hash
 * 
 * @param path 文件名
 * @return uint32_t hash
 */
uint32_t path_hash(const char *path)
{
    uint32_t seed = 1313; // 31, 131, 1313, 13131, 131313, etc...
    uint32_t hash = 0;

    while(*path){
        hash = hash * seed + (*(path++));
    }

    return hash;
}

/**
 * @brief 
 * 
 * @param path 
 * @param mode 
 * @return int handlerNum??
 */
int open(const char *path, int mode)
{
    int index, handlerNum = -1;
    FRESULT res;

    FF_LOCK();

    for(index = 0; index < FILE_MAX; ++index) {
        if(_g_file_opened[index].fileUsing != FILE_USING) {
            handlerNum = index;
            break;
        }
    }

    if(index >= FILE_MAX) {
        LOG_E("FileSys, no valid file object to open a new file, path:%s\r\n", path);
        FF_UNLOCK();
        // ASSERT(0);
        return -FR_TOO_MANY_OPEN_FILES;
    }

    // int -> BYTE, 高三字节省略????
    res = f_open(_g_file_opened[index].fileHandler, path, (BYTE)mode);
    if(res != FR_OK) {
        LOG_E("open file %s failed, res:%d\r\n", path, res);
        return -res;
    }

    ++_g_file_opened_num;
    _g_file_opened[handlerNum].fileUsing = FILE_USING;
    _g_file_opened[handlerNum].fileTimeStamp = FF_GET_SYS_TICK();
    strncpy((char*)_g_file_opened[handlerNum].fileName, path, _MAX_LFN - 1);
    _g_file_handle_base += HANDLE_INCREASE_STEP;
    if(_g_file_handle_base < 0 || (_g_file_handle_base + handlerNum < 0)) {
        _g_file_handle_base = 0;
    }
    _g_file_opened[handlerNum].filePathHash = path_hash(path); // compute hash
    _g_file_opened[handlerNum].fileHandle = _g_file_handle_base + handlerNum;
    handlerNum = _g_file_handle_base + handlerNum;

    printf("filePathHash:%d, fileHandle:%d, _g_file_handle_base:%d\r\n", _g_file_opened[handlerNum].filePathHash, \
                                                                        _g_file_opened[handlerNum].fileHandle, \
                                                                        _g_file_handle_base);

    FF_UNLOCK();

    return handlerNum;
}

int close(int handle)
{
    FRESULT res;
    int uniq_handle = handle;
    handle = handle % HANDLE_INCREASE_STEP;

    if(handle < 0 || handle >= FILE_MAX) {
        LOG_E("close err param:%d\r\n", handle);
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();
    if(uniq_handle != _g_file_opened[handle].fileHandle) {
        LOG_E("File %s close failed, param:%d\r\n", _g_file_opened[handle].fileName, uniq_handle);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    if(_g_file_opened[handle].fileUsing == FILE_USING) {
        res = f_close(_g_file_opened[handle].fileHandler);
        if(res != FR_OK) {
            // res = yaffsfs_GetError();
            LOG_E("File %s close failed, res:%d\r\n", _g_file_opened[handle].fileName, res);
            FF_UNLOCK();
            return res;
        }
        memset(&_g_file_opened[handle], 0, sizeof(fs_file_t));
        if(_g_file_opened_num) {
            --_g_file_opened_num;
        }
    }

    FF_UNLOCK();

    return FR_OK;
}

/**
 * @brief 根据指定大小读取文件内容到缓冲区
 * 
 * @param handle 文件对象句柄(handle), 通过这个句柄可以计算出文件对象索引
 * @param buff 
 * @param len 
 * @return int 错误返回负数, ff.h
 *             成功返回实际读取到的字节数
 */
int read(int handle, void *buff, int len)
{
    UINT byteRead = 0;
    int uniq_handle = handle;
    FRESULT res;
    
    handle = handle % HANDLE_INCREASE_STEP; // 计算索引:0 - FILE_MAX

    if(handle < 0 || handle > FILE_MAX) {
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    if(uniq_handle != _g_file_opened[handle].fileHandle) {
        LOG_E("File read failed, handle:%d, index:%d\r\n", uniq_handle, handle);
        FF_UNLOCK();
        return -FR_INVALID_PARAMETER;
    }

    res = f_read(_g_file_opened[handle].fileHandler, buff, len, &byteRead);
    if(res != FR_OK) {
        LOG_E("file %s read failed, res:%d\r\n", _g_file_opened[handle].fileName, res);
        FF_UNLOCK();
        return -res;
    }

    FF_UNLOCK();

    return byteRead;
}

/**
 * @brief read and sync
 * 
 * @param handle 
 * @param buff 
 * @param len 
 * @return int 
 */
int read_and_sync(int handle, uint8_t *buff, int len)
{
    int i;
    int byteRead, byteTotal;
    int uniq_handle = handle;
    FRESULT res;

    handle = handle % HANDLE_INCREASE_STEP;

    if(handle < 0 || handle >= FILE_MAX) {
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    if(uniq_handle != _g_file_opened[handle].fileHandle) {
        LOG_E("File %d read_and_sync invalid %d\r\n", handle, uniq_handle);
        FF_UNLOCK();
        return -FR_INVALID_PARAMETER;
    }

    byteTotal = 0;
    for(i = 0; i < 5; ++i) { // 总共尝试 5 次
        res = f_read(_g_file_opened[handle].fileHandler, (void*)&buff[byteTotal], len - byteTotal, &byteRead);
        if(res != FR_OK) {
            FF_UNLOCK();
            return byteTotal;
        }

        byteTotal += byteRead;

        if(byteTotal == len) {
            break;
        }
    }

    FF_UNLOCK();

    return byteTotal;
}


/**
 * @brief 从指定偏移量开始, 读取 len byte 数据到 buffer
 * 
 * @param handle 
 * @param offset 
 * @param buff 
 * @param len 
 * @return int 
 */
int read_seek(int handle, int64_t offset, void *buff, int len)
{
    int uniq_handle = handle;
    int64_t res;

    handle = handle % HANDLE_INCREASE_STEP;

    if(handle < 0 || handle >= FILE_MAX) {
        return -FR_INVALID_PARAMETER;
    }

    if(uniq_handle != _g_file_opened[handle].fileHandle) {
        LOG_E("File %d read seek invalid %d\r\n", handle, uniq_handle);
        return -FR_INVALID_OBJECT;
    }

    FF_LOCK();

    res = f_lseek(_g_file_opened[handle].fileHandler, offset); // lseek() 本身就是从起点开始 SEEK_SET
    if(res < 0) {
        // res = fs_get_error;
        LOG_E("file %s seek error %d", _g_file_opened[handle].fileName, res);
        FF_UNLOCK();
        return (int)res;
    }

    FF_UNLOCK();

    return read_and_sync(uniq_handle, buff, len);
}

/**
 * @brief 打开文件, 从指定偏移量开始, 按照指定大小读取文件内容到缓冲区, 并关闭文件
 * 
 * @param path 
 * @param mode 
 * @param offset 
 * @param buff 
 * @param len 
 * @return int 错误返回值, FRESUT
 *             实际读取到的字节数
 */
int open_read_seek(const char *path, int mode, int64_t offset, void *buff, int len)
{
    int byteRead;
    int handle = open(path, mode);
    FRESULT res;

    if(handle < 0){
        LOG_E("file %s open failed %d\r\n", path, handle);
        return handle; // FRESULT type
    }

    byteRead = read_seek(handle, offset, buff, len);

    res = close(handle);
    if(res < 0){
        LOG_E("file %s open failed %d\r\n", path, res);
        return res;
    }

    return byteRead;
}

/**
 * @brief 写文件
 * 
 * @param handle 
 * @param buff 
 * @param len 
 * @return int 
 */
int write(int handle, const void *buff, int len)
{
    FRESULT res;
    int byteWrite = 0;
    int uniq_handle = handle;
    handle = handle % HANDLE_INCREASE_STEP;

    if(handle < 0 || handle >= FILE_MAX || len == 0) {
        LOG_E("write failed, invalid parameter\r\n");
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    if(uniq_handle != _g_file_opened[handle].fileHandle) {
        LOG_E("file %d write invalid %d", handle, uniq_handle);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    res = f_write(_g_file_opened[handle].fileHandler, buff, len, &byteWrite);
    if(res  != FR_OK) {
        LOG_E("file %s write failed:%d\r\n", _g_file_opened[handle].fileName, res);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    FF_UNLOCK();

    return byteWrite;
}

int write_and_sync(int handle, const uint8_t *buff, int len)
{
    int i;
    FRESULT res;
    int byteWrite, byteTotal;
    int uniq_handle = handle;


    handle = handle % HANDLE_INCREASE_STEP;

    if(handle < 0 || handle >= FILE_MAX) {
        LOG_E("File %d invalid parameter %d\r\n", handle, uniq_handle);
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    if(uniq_handle != _g_file_opened[handle].fileHandle) {
        LOG_E("File %d Invalid parameter %d\r\n", handle, uniq_handle);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    byteTotal = 0;

    for(i = 0; i < 5; ++i) {
        res = f_write(_g_file_opened[handle].fileHandler, &buff[byteTotal], len - byteTotal, &byteWrite);
        if(res != FR_OK) {
            LOG_E("Write file %s failed:%d\r\n", _g_file_opened[handle].fileName, res);
            FF_UNLOCK();
            return byteTotal;
        }

        res = f_sync(_g_file_opened[handle].fileHandler);
        if(res != FR_OK) {
            LOG_E("file %s sync failed:%d\r\n", _g_file_opened[handle].fileName, res);
            FF_UNLOCK();
            return res;
        }

        byteTotal += byteWrite;
        if(byteTotal == len) {
            break;
        }
    }

    FF_UNLOCK();

    return byteTotal;
}

/**
 * @brief 往文件中追加内容
 * 
 * @param name file name
 * @param buff data buffer
 * @param len data length
 * @return int 返回操左结果
 */
int open_write(const char *name, uint8_t *buff, uint32_t len)
{
    // O_APPEND | O_RDWR
    int fp = open(name, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
    int byteWritten = 0;
    int res = 0;

    if(fp < 0) {
        LOG_E("file %s open failed\r\n", name, fp);
        return fp;
    }

    byteWritten = write(fp, buff, len);
    if(byteWritten < 0){
        LOG_E("file %s write failed:%d\r\n", name, byteWritten);
    }

    res = close(fp);
    if(res < 0) {
        LOG_E("file %s close faile:%d\r\n", name, res);
        return byteWritten;
    }

    return FR_OK;
}

void close_opened_file(const char *path)
{
    FRESULT res;
    uint32_t hash;
    uint32_t index;

    if(!path) 
        return;
    
    hash = path_hash(path); // 计算 hash 值

    FF_LOCK();

    for(index = 0; index < FILE_MAX; ++index) {
        if(_g_file_opened[index].fileUsing == FILE_USING) {
            if(_g_file_opened[index].filePathHash == hash) {
                res = f_close(_g_file_opened[index].fileHandler);
                if(res != FR_OK) {
                    LOG_E("File %s close failed:%d\r\n", path, res);
                }
                memset(&_g_file_opened[index], 0, sizeof(fs_file_t));
                if(_g_file_opened_num != 0) {
                    --_g_file_opened_num;
                }
            }
        }
    }
}

int sync(int handle)
{
    FRESULT res;
    int uniq_handle = handle;

    handle = handle % HANDLE_INCREASE_STEP;

    if(handle < 0 || handle >= FILE_MAX) {
        LOG_E("file sync failed, handle:%d\r\n", handle);
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    if(uniq_handle != _g_file_opened[handle].fileHandle) {
        LOG_E("File %d sync invalid %d\r\n", handle, uniq_handle);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    res = f_sync(_g_file_opened[handle].fileHandler);
    if(res != FR_OK){
        //res = yaffsfs_get_error();
        LOG_E("File %s sync failed res:%d\r\n", _g_file_opened[handle].fileName, res);
        FF_UNLOCK();
        return res;
    }

    FF_UNLOCK();

    return res;
}

/**
 * @brief 文件删除
 * 
 * @param path 
 * @return int 错误返回负值
 *             成功返回 FR_OK
 */
int unlink(const char *path)
{
    FRESULT res;
    if(!path){
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    res = f_unlink(path);
    if(res < 0) {
        // res = yaffsfs_get_error();
        LOG_E("unlink file %s failed %d\r\n", path, res);
        FF_UNLOCK();
        return res;
    }

    FF_UNLOCK();

    return res;
}

/**
 * @brief 定位文件指针到指定位置
 * 
 * @param handle 
 * @param offset 
 * @param whence SEEK_SET - 从文件头偏移
 *               SEEK_CUR - 从当前位置偏移
 *               SEEK_END - 从文件结束位置偏移
 * @return int64_t 成功返回文件指针位置
 *                 错误返回错误类型
 *                 这里与 yaffs 不太一样, 这里仅仅返回是否成功, 文件偏移指针在文件系统内部维护
 */
int lseek(int handle, int64_t offset, uint8_t whence)
{
    int64_t pos;
    int uniq_handle = handle;
    FRESULT res;

    handle = handle % HANDLE_INCREASE_STEP;

    if(handle < 0 || handle >= FILE_MAX) {
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    if(uniq_handle != _g_file_opened[handle].fileHandle) {
        LOG_E("File %d lseek invvalid %d\r\n", handle, uniq_handle);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    // 固定从文件头部开始偏移
    res = f_lseek(_g_file_opened[handle].fileHandler, offset);
    if(res != FR_OK) {
        LOG_E("File %d seek failed %d res:%d\r\n", handle, uniq_handle, res);
        FF_UNLOCK();
        return -FR_INVALID_PARAMETER;
    }

    FF_UNLOCK();

    return res;
}

int64_t fsize(int handle)
{
    int64_t ret;
    int uniq_handle = handle;
    FIL *fp;

    handle = handle % HANDLE_INCREASE_STEP;

    if(handle < 0 || handle >= FILE_MAX) {
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    if(uniq_handle != _g_file_opened[handle].fileHandle) {
        LOG_E("File %d lseek invvalid %d\r\n", handle, uniq_handle);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    fp = _g_file_opened[handle].fileHandler;

    if(fp != NULL){
        printf("file %d invalid %d\r\n", handle, uniq_handle);
        return -FR_INVALID_OBJECT;
    }

    return f_size(fp);
}

/**
 * @brief 判断是否文件结束
 * 
 * @param handle 
 * @return int 1  文件结束
 *             0  未结束
 *             -1 错误
 */
int fileeof(int handle)
{
    int ret = -1;
    int uniq_handle = handle;

    handle = handle % HANDLE_INCREASE_STEP;

    if(handle < 0 || handle >= FILE_MAX) {
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    if(uniq_handle != _g_file_opened[handle].fileHandle) {
        LOG_E("File %d feof failed %d\r\n", handle, uniq_handle);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    return f_eof(_g_file_opened[handle].fileHandler);
}

/**
 * @brief 按照当前 offset 阶段文件
 * 
 * @param handle 
 * @return int FR_OK
 *             error code
 */
int ftruncate(int handle)
{
    int res;
    FIL *fp = NULL;
    int uniq_handle = handle;

    handle = handle % HANDLE_INCREASE_STEP;

    if(handle < 0 || handle >= FILE_MAX) {
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    if(uniq_handle != _g_file_opened[handle].fileHandle) {
        LOG_E("File %d lseek invvalid %d\r\n", handle, uniq_handle);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    fp = _g_file_opened[handle].fileHandler;
    if(fp != NULL) {
        printf("file %d invalid %d\r\n", handle, uniq_handle);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    res = f_truncate(fp);
    if(res != FR_OK) {
        // res = fatfs_get+_error();
        LOG_E("truncate %d failed %d, res:%d\r\n", handle, uniq_handle, res);
        FF_UNLOCK();
        return res;
    }

    FF_UNLOCK();

    return res;
}

/**
 * @brief 获取磁盘剩余容量
 * 
 * @param pDrv 当前磁盘:0, 1... 
 * @return uint32_t 获取磁盘剩余容量:KB
 *                  返回错误代码
 */
int fgetfree(char *pDrv)
{
    DWORD byteFree;
    FRESULT res;
    
    // FATFS 的对象应该存在哪里
    res = f_getfree("0:", &byteFree, &fs[0]);
    if(res != FR_OK) {
        LOG_E("File System get free failed %d\r\n", res);
        return FR_INVALID_PARAMETER;
    }
    return (uint32_t)byteFree;
}

/**
 * @brief 获取磁盘总容量
 * 
 * @param pDrv 
 * @return uint32_t 磁盘总容量 KB
 */
uint32_t fgettotal(char *pDrv)
{
    uint64_t byteTotal = 0;

    // f_get_total(pDrv)

    return byteTotal;
}

/**
 * @brief 获取文件状态
 * 
 * @param path 
 * @param[out] fstat 状态句柄指针, 输出参数
 * @return int 
 */
int fstat(const char *path, void *fstat)
{
    FRESULT res;

    FF_LOCK();

    res = f_stat(path, (FILINFO*)fstat);

    if(res < 0) {
        // res = yafs_get_error();
        LOG_E("File %s state failed, res:%d\r\n", path, res);
        return res;
    }

    FF_UNLOCK();

    return res;
}

/**
 * @brief 重命名文件
 * 
 * @param pathOld 
 * @param pathNew 
 * @return int 
 */
int rename(const char *pathOld, const char *pathNew)
{
    FRESULT res;

    if(!pathOld || !pathNew) {
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    res = f_rename(pathOld, pathNew);
    if(res != FR_OK) {
        // get_error();
        LOG_E("File %s rename failed %d\r\n", pathOld, res);
        FF_UNLOCK();
        return res;
    }

    FF_UNLOCK();

    return res;
}

/**
 * @brief 拷贝文件
 * 
 * @param pathOld 
 * @param pathNew 
 * @param inCopyBuff 拷贝的缓存
 * @param inCopyBuffMaxSize 缓存最大值
 * @return int 
 */
int copy_file(const char *pathOld, const char *pathNew, uint8_t *inCopyBuff, uint32_t inCopyBuffMaxSize)
{
    int offset, needReadLen, readLen, writtenLen, size;
    int fpRead, fpWrite;
    if(!pathOld || !pathNew || inCopyBuffMaxSize < 256) {
        return -FR_INVALID_PARAMETER;
    }

    fpRead = open(pathOld, FA_READ);
    if(fpRead < 0) {
        LOG_E("ERR file %s open faile %d\r\n", pathOld, fpRead);
        return fpRead;
    }

    size = fsize(fpRead);

    fpWrite = open(pathNew, FA_WRITE);
    if(fpWrite < 0) {
        LOG_E("ERR file %s open faile %d\r\n", pathOld, fpWrite);
        return fpWrite;
    }

    offset = 0;
    while(offset < size) {
        if((offset + inCopyBuffMaxSize) < size) {
            needReadLen = inCopyBuffMaxSize;
        }
        else {
            needReadLen = size - offset;
        }

        readLen = read_and_sync(fpRead, inCopyBuff, needReadLen);
        if(readLen != needReadLen) {
            LOG_E("copy old file %s error, offset:%d %d != %d", pathOld, offset, readLen, needReadLen);
            close(fpRead);
            close(fpWrite);
            return -FR_DISK_ERR;
        }

        writtenLen = write_and_sync(fpWrite, inCopyBuff, readLen);
        if(writtenLen != readLen) {
            LOG_E("write new file %s error, offset:%d %d != %d", pathNew, offset, writtenLen, readLen);
            close(fpRead);
            close(fpWrite);
            return -FR_DISK_ERR;
        }

        offset += writtenLen;
    }

    close(fpRead);
    close(fpWrite);

    return FR_OK;
}

/**
 * @brief 校验路径
 * 
 * @param path 文件夹路径
 * @param temp 初始化 temp 值用于拼接更长的路径
 * @return int FR_OK or other err code
 */
static int path_verify(const char *path, char temp[2])
{
    int len;

    if(!path || !temp) {
        return -FR_INVALID_PARAMETER;
    }

    len = strlen(path);

    if(len < strlen(ROOT_DIR)) { // 目录以 "0:" 开始, 所以长度需要大于 2
        return -FR_INVALID_PARAMETER;
    }

    return FR_OK;
}


// directory operation
/**
 * @brief 创建目录
 * 
 * @param path 
 * @return int 
 */
int mkdir(const char *path)
{
    FRESULT res;

    FF_LOCK();

    res = f_mkdir(path);
    if(res != FR_OK) {
        // get_error();
        LOG_E("dir %s create failed %d\r\n", path, res);
        FF_UNLOCK();
        return res;
    }

    FF_UNLOCK();

    return res;
}

/**
 * @brief 递归创建目录
 *        Linux cmd: mkdir -p
 * 
 * @param path 绝对路径, 例如:/nand/sys/test.txt
 * @return int FRESTULT
 */
int mkdir_p(const char *path)
{
    FRESULT res;
    const char *pTemp;
    const char *pNext;
    char name[128];
    int handle;
    int length;

    if(!path) {
        LOG_E("path %s error\r\n", path);
        return -FR_INVALID_PARAMETER;
    }

    pNext = path;
    if(pNext[0] == '/') { // 跳过第一个 '/'
        pNext++;
    }

    // do {
    //     pTemp = strchr((const char *)pNext, (int)'/'); // pTemp -> "/sys/test.txt"
    //     if(pTemp == NULL) {
    //         length = strlen(path);
    //         pNext = NULL;
    //     }
    //     else {
    //         pNext = pTemp + 1;
    //         length = pTemp - path; // 获取 "nand" 的长度
    //     }
    //     memcpy(name, path, length);
    //     name[length] = '\0';
    //     handle = opendir(name); // 打开文件夹
    //     if(handle < 0) { // 打开文件夹失败, 说明文件不存在
    //         res = (FRESULT)mkdir((const char*)name);
    //         if(res != FR_OK) {
    //             return res;
    //         }
    //     }
    //     else { // 打开文件夹成功, 说明文件已经存在, 不需要创建
    //         closedir(handle);
    //     }
    //     // memset(name, 0, sizeof(name));
    // }
    // while(pNext != NULL);

    return FR_OK;
}

/**
 * @brief 打开目录
 * 
 * @param path 绝对路径
 * @return int FRESTULT
 */
int opendir(const char *path)
{
    int index, handlerNum;
    FRESULT res;

    FF_LOCK();

    if(_g_dir_opened_num > DIR_MAX) {
        LOG_E("open maximum dir num:%d\r\n", _g_dir_opened);
        FF_UNLOCK();
        return -FR_INVALID_PARAMETER;
    }

    // 有效目录对象检查
    for(index = 0; index < DIR_MAX; ++index) {
        if(!_g_dir_opened[index].dirUsing) {
            handlerNum = index;
            break;
        }
    }
    if(index >= DIR_MAX) {
        LOG_E("no valid dir object\r\n");
        FF_UNLOCK();
        return -FR_TOO_MANY_OPEN_FILES;
    }

    res = f_opendir(_g_dir_opened[handlerNum].dirHandler, path);

    if(!_g_dir_opened[handlerNum].dirHandler) {
        // res = get_error();
        LOG_E("DIR %s open failed %d\r\n", path, res);
        FF_UNLOCK();
        return res;
    }

    ++_g_dir_opened_num;
    _g_dir_opened[handlerNum].dirUsing = FILE_USING; // DIR_USING
    _g_dir_opened[handlerNum].dirTimeStamp = FF_GET_SYS_TICK();
    strncpy((char*)_g_dir_opened[handlerNum].dirName, path, _MAX_LFN - 1);
    _g_dir_handle_base += HANDLE_INCREASE_STEP;
    if(_g_dir_handle_base < 0 || (_g_dir_handle_base + handlerNum < 0)) {
        _g_dir_handle_base = 0;
    }
    _g_dir_opened[handlerNum].dirPathHash = path_hash(path);
    _g_dir_opened[handlerNum].dirHandle = _g_dir_handle_base + handlerNum;
    handlerNum = _g_dir_handle_base + handlerNum;

    FF_UNLOCK();

    return handlerNum;
}

/**
 * @brief 
 * 
 * @param handle 
 * @return int 
 */
int closedir(int handle)
{
    FRESULT res;
    int uniq_handle = handle;

    handle = handle % HANDLE_INCREASE_STEP;
    if(handle < 0 || handle >= DIR_MAX) {
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    if(uniq_handle != _g_dir_opened[handle].dirHandle) {
        LOG_E("Dir %d close dir invalid", handle, uniq_handle);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    res = f_closedir(_g_dir_opened[handle].dirHandler);
    if(res != FR_OK) {
        // get error
        LOG_E("Dir:%s close failed:%d\r\n", _g_dir_opened[handle].dirName);
        FF_UNLOCK();
        return res;
    }

    memset(&_g_dir_opened[handle], 0, sizeof(fs_dir_t));
    if(_g_dir_opened_num) {
        --_g_dir_opened_num;
    }

    FF_UNLOCK();

    return FR_OK;
}

/**
 * @brief 目录读取
 * 
 * @param handle 
 * @param info 普通镜指针
 * @return int FRESULT
 */
int readdir(int handle, void *info)
{
    FRESULT res;
    int uniq_handle = handle;
    void **ret;

    handle = handle % HANDLE_INCREASE_STEP;
    if(handle < 0 || handle >= DIR_MAX || !info) {
        return -FR_INVALID_PARAMETER;
    }

    FF_LOCK();

    if(uniq_handle != _g_dir_opened[handle].dirHandle) {
        LOG_E("Dir %d close dir invalid", handle, uniq_handle);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    // ret = (void**)info; // 二重指针

    res = f_readdir(_g_dir_opened[handle].dirHandler, info);
    if(res != FR_OK) {
        LOG_E("dir:%s read failed:%d\r\n", _g_dir_opened[handle].dirName, res);
        FF_UNLOCK();
        return -FR_INVALID_OBJECT;
    }

    FF_UNLOCK();

    return FR_OK;
}

int recursively_delete(const char *path, int rmFileOnly)
{

}

/**
 * @brief 先判断有没有打开, 如果打开, 先关闭, 然后删除
 * 
 * @param path 
 * @param rmFileOnly 
 * @return int 
 */
int rmdir(const char *path, int rmFileOnly)
{
    int index;
    FRESULT res;
    bool inUsing = false;;

    FF_LOCK();
    // 判断目录是否已经被打开
    for(index = 0; index < DIR_MAX; ++index) {
        if(strcmp(_g_dir_opened[index].dirName, path) == 0) {
            if(_g_dir_opened[index].dirUsing == DIR_USING) {
                inUsing = true;
            }
        }
    }

    if(inUsing) {
        res = f_closedir(_g_dir_opened[index].dirHandler);
        if(res != FR_OK) {
            LOG_E("dir %s close failed %d\r\n", _g_dir_opened[index].dirName, res);
            FF_UNLOCK();
            return -FR_INVALID_OBJECT;
        }
    }

    f_unlink(_g_dir_opened[index].dirName); // 删除文件或目录都用这个接口

    memset(&_g_dir_opened[index], 0, sizeof(fs_dir_t));

    FF_UNLOCK();

    return FR_OK;
}

/**
 * @brief 查找文件是否存在
 * 
 * @param path 
 * @return int 0:存在; 非零:不存在
 */
int find_dir(const char *path)
{
    int handle;

    if(!path) {
        LOG_E("ERROR parameter\r\n");
        return -FR_INVALID_PARAMETER;
    }

    handle = opendir(path);

    if(handle < 0) { // 文件不存在
        return -1;
    }
    closedir(handle);

    return 0;
}

void close_opened_dir(const char *path)
{
    uint32_t hash;
    uint32_t index;
    FRESULT res;
    if(!path) {
        return;
    }

    hash = path_hash(path);
    FF_LOCK();
    for(index = 0; index < DIR_MAX; ++index) {
        if(_g_dir_opened[index].dirUsing) {
            if(_g_dir_opened[index].dirPathHash == hash) {
                res = f_closedir(_g_dir_opened[index].dirHandler);
                if(res != FR_OK) {
                    LOG_E("dir %s close failed %d\r\n", _g_dir_opened[index].dirName, res);
                    memset(&_g_dir_opened[index], 0, sizeof(fs_dir_t));
                    if(_g_dir_opened_num) {
                        --_g_dir_opened_num;
                    }
                }
            }
        }
    }
    FF_UNLOCK();

    return;
}

static int copy_folder(const char *path, const char *pathDst, buffer_area_t *temp_data)
{
    FRESULT res = FR_OK;
    int dirFp;
    char temp[2];
    char pathName[_MAX_LFN];
    char tempDst[2];
    char pathNameDst[_MAX_LFN];
    FILINFO *pInfo = NULL;

    res = (FRESULT)path_verify(path, temp);
    if(res != FR_OK) {
        LOG_E("Error path:%s", path);
        return res;
    }
    res = (FRESULT)path_verify(pathDst, temp);
    if(res != FR_OK) {
        LOG_E("Error pathDst:%s", pathDst);
        return res;
    }

    dirFp = opendir(path);
    if(dirFp < 0) {
        return -FR_INVALID_PARAMETER;
    }

    pInfo = mymalloc(sizeof(FILINFO) * 1);
    if(!pInfo) {
        LOG_E("memory allocate failed.\r\n");
        return -1;
    }
    pInfo->lfsize = _MAX_LFN; // 设置文件名的最大长度
    pInfo->lfname = (TCHAR*)mymalloc(pInfo->lfsize);
    if(!pInfo->lfname) {
        LOG_E("memory allocate failed.\r\n");
        myfree(pInfo);
        return -2;
    }

    while((res = f_readdir(_g_dir_opened[dirFp].dirHandler, pInfo)) == FR_OK) {
        // source directory name
        res = snprintf(pathName, _MAX_LFN, "%s/%s", path, pInfo->fname);
        if(res > _MAX_LFN) {
            res = -FR_INVALID_NAME;
            break;
        }
        // copy directory name
        res = snprintf(pathNameDst, _MAX_LFN, "%s/%s", pathDst, pInfo->fname);
        if(res > _MAX_LFN) {
            res = -FR_INVALID_NAME;
            break;
        }

        // 判断是文件还是文件夹
        // 然后进行文件拷贝/文件夹拷贝
        // copyu_folder();
        // copy_file();
    }

    closedir(dirFp);
    myfree(pInfo->lfname);
    myfree(pInfo);

    return FR_OK;
}

/**
 * @brief 备份目录:目标文件存在时, 先删除目标目录, 然后重新创建空的目标目录, 拷贝
 * 
 * @param old_path 
 * @param new_path 
 * @param temp_data 
 * @return int 
 */
int cut_folder(const char *old_path, const char *new_path, buffer_area_t *temp_data)
{

}

/**
 * @brief 遍历目录:获取文件名列表
 * 
 * @param p_root_path 文件夹路径
 * @param name_list 文件名列表
 * @param list_len 文件个数
 * @return int 
 */
int file_tree(const char *p_root_path, char name_list[][128], uint32_t list_len)
{
    FRESULT res;
    int dir_fp;
    FILINFO *pInfo = NULL;
    uint32_t file_cnt = 0;

    dir_fp = opendir(p_root_path);
    if(dir_fp < 0) {
        LOG_E("dir:%s oepn failed\r\n", p_root_path);
        return dir_fp;
    }

    // allocate memory
    pInfo = mymalloc(sizeof(FILINFO) * 1);
    if(!pInfo) {
        LOG_E("memory allocate failed.\r\n");
        return -1;
    }
    pInfo->lfsize = _MAX_LFN; // 设置文件名的最大长度
    pInfo->lfname = (TCHAR*)mymalloc(pInfo->lfsize);
    if(!pInfo->lfname) {
        LOG_E("memory allocate failed.\r\n");
        myfree(pInfo);
        return -2;
    }

    while(1) {
        res = readdir(dir_fp, (void*)pInfo);
        if(res != FR_OK) {
            break;
        }

        if(snprintf(name_list[file_cnt], 128, "%s/%s", p_root_path, (const char *)pInfo->lfname) > _MAX_LFN - 1) {
            LOG_E("truncation! %s/%s", p_root_path, (const char*)pInfo->lfname);
            continue;
        }
        ++file_cnt;
        if(file_cnt > list_len) {
            break;
        }
    }

    closedir(dir_fp);
    myfree(pInfo->lfname);
    myfree(pInfo);

    return FR_OK;
}

void fs_print_opened_files(fs_dump_cb_t cb)
{
    int index = 0;
    if(!cb){
        LOG_E("ERROR null cb");
        return;
    }

    for(index = 0; index < DIR_MAX; ++index) {
        cb( FS_FILE_SYSTEM_NAME,
            index, 
           _g_dir_opened[index].dirTimeStamp, 
           _g_dir_opened[index].dirUsing ? "Using" : "Not using",
           _g_dir_opened[index].dirUsing ? _g_dir_opened[index].dirName : "NA");
    }

    for(index = 0; index < DIR_MAX; ++index) {
        cb(FS_FILE_SYSTEM_NAME,
           index, 
           _g_file_opened[index].fileTimeStamp, 
           _g_file_opened[index].fileUsing ? "Using" : "Not using",
           _g_file_opened[index].fileUsing ? _g_file_opened[index].fileName : "NA");
    }

    return;
}

/**
 * @brief 打印目录树
 * 
 * @param dname 
 * @param recursive 目录深度
 */
static void dump_directory_tree_worker(const char *dname, int recursive)
{

}

/**
 * @brief 打印以及目录
 * 
 * @param path 
 */
void dump_directory_tree(const char *path)
{
    dump_directory_tree_worker(path, 1);
}




