/**
 * @file fattester.c
 * @author your name (you@domain.com)
 * @brief 对 FATFS 接口的封装, 一个简单地测试代码，不足以作为中间层, 
 *        真正要用还是要自己直接封装 FATFS 的接口
 * @version 0.1
 * @date 2023-01-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "fattester.h"	 
// #include "mmc_sd.h"
#include "usmart.h"
#include "usart.h"
#include "exfuns.h"
#include "malloc.h"
#include "ff.h"
#include "string.h"

// 标准 C 库文件操作相关 API 介绍:https://cplusplus.com/reference/cstdio/fopen/
// FATFS API: http://elm-chan.org/fsw/ff/doc/mkfs.html
// Linux 系统调用 API:https://linux.die.net/man/2/truncate


// 仅用于测试
// TODO: 
// 暂不支持中文文件名, 打开后代码段过大导致编译不过


/*********************************************************************************
 * PROTOTYPE
 *********************************************************************************/
typedef struct {
    const char *name; // test item name
    int (*op)(void); // test item operation
} fs_op_test_item_t;

typedef struct {
    const char *fs_name;
    fs_op_test_item_t start_test; // 起点
    /* other test begin*/
    fs_op_test_item_t open_test;
    fs_op_test_item_t async_test;
    fs_op_test_item_t str_op_test;
    fs_op_test_item_t file_locate_test;
    fs_op_test_item_t get_file_info_test;
    fs_op_test_item_t file_end_test;
    fs_op_test_item_t allocate_sequential_space_test;
    fs_op_test_item_t read_transfer_test;
    
    fs_op_test_item_t dir_op_open;
    fs_op_test_item_t dir_op_create;
    fs_op_test_item_t dir_op_change_curr;
    fs_op_test_item_t dir_op_change_attr;

    /* other test end*/
    fs_op_test_item_t end_test; // 结束
} fs_op_test_manager;
/*********************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************/
static int start_fs_op_test(void);
static int end_fs_op_test(void);
static int file_open_test(void);
static int file_op_async_test(void);
static int file_op_string_input_test(void);
static int file_op_locate_test(void);
static int file_op_get_file_info_test(void);
static int file_op_file_end_test(void);
static int file_op_allocate_sequential_space_test(void);
static int file_op_read_transfer_test(void);
static int dir_op_open_test(void);
static int dir_create_test(void);
static int dir_change_curr_test(void);
static int dir_modify_attr_test(void);

/*********************************************************************************
 * PRIVATE VARIABLES
 *********************************************************************************/
// NOTE: ANSI C 结构体的这种赋值方法编译不过, 切换为 C99 后可以编译过
static fs_op_test_manager fatfs_test_manager = {
    .fs_name = "FATFS",
    .start_test = {"start_test", start_fs_op_test},
    .end_test = {"end_test", end_fs_op_test},
    .async_test = {"async_test", file_op_async_test},
    .open_test = {"open_test", file_open_test},
    .str_op_test = {"str_op_test", file_op_string_input_test},
    .file_locate_test = {"file_locate_test", file_op_locate_test},
    .get_file_info_test = {"get_file_info_test", file_op_get_file_info_test},
    .file_end_test = {"file_end_test", file_op_file_end_test},
    .allocate_sequential_space_test = {"allocate_sequential_space_test", file_op_allocate_sequential_space_test},
    .read_transfer_test = {"read_transfer_test", file_op_read_transfer_test},
    .dir_op_open = {"dir_op_open", dir_op_open_test},
    .dir_op_create = {"dir_op_create", NULL}, // dir_create_test
    .dir_op_change_curr = {"dir_op_change_curr", NULL}, // dir_change_curr_test
    .dir_op_change_attr = {"dir_op_change_attr", dir_modify_attr_test},
};

/**
 * @brief 为磁盘注册工作区(挂载)
 * 
 * @param path 磁盘路径，比如"0:"、"1:"
 * @param mt 0，不立即注册（稍后注册）；1，立即注册
 * @return u8 执行结果
 */
u8 mf_mount(u8* path, u8 mt)
{
//    return f_mount(fs[0], (const TCHAR*)path, mt); 
    FATFS *pFS;
    switch (path[0]) {
        case '0':
        {
            pFS = fs[0];
        }
        break;
        case '1':
        {
            pFS = fs[1];
        }
        break;
        default:
            printf("Error, physical volume:%s\r\n", path);
        break;
    }
    return f_mount(pFS, (const TCHAR*)path, mt); 
}

/**
 * @brief 打开路径下的文件
 * 
 * @param path 路径+文件名
 * @param mode 打开模式
 * @return u8 执行结果
 */
u8 mf_open(u8*path, u8 mode)
{
    // u8 res;
    // res = f_open(file, (const TCHAR*)path, mode);//打开文件夹
    // return res;
    return (u8)f_open(file, (const TCHAR*)path, mode); //打开文件夹
}

/**
 * @brief 关闭文件
 * 
 * @return u8 执行结果
 */
u8 mf_close(void)
{
    return (u8)f_close(file);
}

/**
 * @brief 读出数据
 * 
 * @param len 读出的长度
 * @return u8 执行结果
 */
u8 mf_read(u16 len)
{
    u16 i, t;
    u8 res = 0;
    u16 tlen = 0;
    printf("\r\nRead file data is:\r\n");
    for(i = 0; i < len / 512; i++)
    {
        // 为什么要这么操作, 只能 512 bytes????
        // TODO:
        res = f_read(file, fatbuf, 512, &br);
        printf("read len1:%d, ret:%d\r\n", br, res);
        if(res)
        {
            printf("Read Error:%d\r\n",res);
            break;
        }
        else {
            tlen += br;
            for(t = 0; t < br; t++)
                printf("%c", fatbuf[t]); 
        }
    }
    if(len % 512)
    {
        res = f_read(file, fatbuf, len % 512, &br);
        printf("read len2:%d, ret:%d\r\n", br, res);

        if(res)	//读数据出错了
        {
            printf("\r\nRead Error:%d\r\n", res);   
        }
        else{
            tlen+=br;
            for(t = 0; t<br; t++)
                printf("%c", fatbuf[t]); 
        }	 
    }
    if(tlen)
        printf("\r\nReaded data total len:%d\r\n", tlen);//读到的数据长度
    // printf("Read data over\r\n");	 
    return res;
}

/**
 * @brief 写入数据
 * 
 * @param dat 数据缓存区
 * @param len 写入长度
 * @return u8 执行结果
 */
u8 mf_write(u8*dat, u16 len)
{
    u8 res;	   					   

    printf("\r\nBegin Write file...\r\n");
    printf("Write data len:%d\r\n", len);	 
    res = f_write(file, dat, len, &bw);
    if(res){
        printf("Write Error:%d\r\n", res);   
    }
    else 
        printf("Writed data len:%d\r\n", bw);
    printf("Write data over.\r\n");
    return res;
}

//打开目录
//path:路径
//返回值:执行结果
u8 mf_opendir(u8* path)
{
    return f_opendir(&dir, (const TCHAR*)path);	
}

//关闭目录 
//返回值:执行结果
u8 mf_closedir(void)
{
    return f_closedir(&dir);	
}

//打读取文件夹
//返回值:执行结果
u8 mf_readdir(void)
{
    u8 res;
    char *fn;			 
#if _USE_LFN
    fileinfo.lfsize = _MAX_LFN * 2 + 1;
    fileinfo.lfname = mymalloc(fileinfo.lfsize);
#endif		  
    res = f_readdir(&dir, &fileinfo);//读取一个文件的信息
    if(res != FR_OK || fileinfo.fname[0] == 0)
    {
        myfree(fileinfo.lfname);
        return res;//读完了.
    }
#if _USE_LFN
    fn=*fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else
    fn=fileinfo.fname;;
#endif	
    printf("\r\n DIR info:\r\n");

    printf("dir.id:%d\r\n", dir.id);
    printf("dir.index:%d\r\n", dir.index);
    printf("dir.sclust:%d\r\n", dir.sclust);
    printf("dir.clust:%d\r\n", dir.clust);
    printf("dir.sect:%d\r\n", dir.sect);	  

    printf("\r\n");
    printf("File Name is:%s\r\n", fn);
    printf("File Size is:%d\r\n", fileinfo.fsize);
    printf("File data is:%d\r\n", fileinfo.fdate);
    printf("File time is:%d\r\n", fileinfo.ftime);
    printf("File Attr is:%d\r\n", fileinfo.fattrib);
    printf("\r\n");
    myfree(fileinfo.lfname);
    return 0;
}

//遍历文件
//path:路径
//返回值:执行结果
u8 mf_scan_files(u8 * path)
{
    FRESULT res;	  
    char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
    fileinfo.lfsize = _MAX_LFN * 2 + 1;
    fileinfo.lfname = mymalloc(fileinfo.lfsize);
#endif		  

    res = f_opendir(&dir, (const TCHAR*)path); //打开一个目录
    if (res == FR_OK) 
    {
        printf("\r\n"); 
        while(1)
        {
            res = f_readdir(&dir, &fileinfo); //读取目录下的一个文件
            if (res != FR_OK || fileinfo.fname[0] == 0)  //错误了/到末尾了,退出
                break;  
            //if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
#if _USE_LFN
            fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else							   
            fn = fileinfo.fname;
#endif	                                              /* It is a file. */
            printf("%s/", path);//打印路径	
            printf("%s\r\n",  fn);//打印文件名
            // 打印完是不是要清空一下 fileinfo?????
            // memset(&fileinfo, 0x00, sizeof(FILINFO));
        }
    }
    myfree(fileinfo.lfname);

    // 没有关闭文件夹

    return res;	  
}

//显示剩余容量
//drv:盘符
//返回值:剩余容量(字节)
u32 mf_showfree(u8 *drv)
{
    FATFS *fs1;
    u8 res;
    u32 fre_clust=0, fre_sect=0, tot_sect=0;
    //得到磁盘信息及空闲簇数量
    res = f_getfree((const TCHAR*)drv,(DWORD*)&fre_clust, &fs1);
    if(res==0)
    {
        tot_sect = (fs1->n_fatent - 2) * fs1->csize;//得到总扇区数
        fre_sect = fre_clust * fs1->csize;			//得到空闲扇区数	   
#if _MAX_SS!=512
        tot_sect*=fs1->ssize/512;
        fre_sect*=fs1->ssize/512;
#endif	  
        if(tot_sect<20480)//总容量小于10M
        {
            /* Print free space in unit of KB (assuming 512 bytes/sector) */
            printf("\r\n磁盘总容量:%d KB\r\n"
                   "可用空间:%d KB\r\n",
                   tot_sect>>1,fre_sect>>1);
        }else
        {
            /* Print free space in unit of KB (assuming 512 bytes/sector) */
            printf("\r\n磁盘总容量:%d MB\r\n"
                   "可用空间:%d MB\r\n",
                   tot_sect>>11,fre_sect>>11);
        }
    }
    return fre_sect;
}

//文件读写指针偏移
//offset:相对首地址的偏移量
//返回值:执行结果.
u8 mf_lseek(u32 offset)
{
    return f_lseek(file,offset);
}

//读取文件当前读写指针的位置.
//返回值:位置
u32 mf_tell(void)
{
    return f_tell(file);
}

//读取文件大小
//返回值:文件大小
u32 mf_size(void)
{
    return f_size(file);
} 

//创建目录
//pname:目录路径+名字
//返回值:执行结果
u8 mf_mkdir(u8 *pname)
{
    return f_mkdir((const TCHAR *)pname);
}

//格式化
//path:磁盘路径，比如"0:"、"1:"
//mode:模式
//au:簇大小
//返回值:执行结果
u8 mf_fmkfs(u8* path, u8 mode, u16 au)
{
    return f_mkfs((const TCHAR*)path, mode, au);//格式化,drv:盘符;mode:模式;au:簇大小
} 

/**
 * @brief 删除文件/目录
 *       TODO:
 *       unlink 删除文件夹的时候必须要保证文件夹为空才可以成功删除
 * 
 * @param pname 文件/目录路径+名字
 * @return u8 执行结果
 */
u8 mf_unlink(u8 *pname)
{
    return  f_unlink((const TCHAR *)pname);
}

//修改文件/目录名字(如果目录不同,还可以移动文件哦!)
//oldname:之前的名字
//newname:新名字
//返回值:执行结果
u8 mf_rename(u8 *oldname,u8* newname)
{
    return  f_rename((const TCHAR *)oldname, (const TCHAR *)newname);
}

//获取盘符（磁盘名字）
//path:磁盘路径，比如"0:"、"1:"  
void mf_getlabel(u8 *path)
{
    u8 buf[20];
    u32 sn = 0;
    u8 res;
    res = f_getlabel ((const TCHAR *)path, (TCHAR *)buf, (DWORD*)&sn);
    if(res == FR_OK)
    {
        printf("\r\n磁盘%s 的盘符为:%s\r\n",path,buf);
        printf("磁盘%s 的序列号:%X\r\n\r\n",path,sn); 
    }
    else
        printf("\r\n获取失败, 错误码:%X\r\n",res);
}

//设置盘符（磁盘名字），最长11个字符！！，支持数字和大写字母组合以及汉字等
//path:磁盘号+名字，比如"0:ALIENTEK"、"1:OPENEDV"  
void mf_setlabel(u8 *path)
{
    u8 res;
    res = f_setlabel ((const TCHAR *)path);
    if(res == FR_OK)
    {
        printf("\r\n磁盘盘符设置成功:%s\r\n",path);
    }
    else 
        printf("\r\n磁盘盘符设置失败, 错误码:%X\r\n",res);

        return;
} 

//从文件里面读取一段字符串
//size:要读取的长度
void mf_gets(u16 size)
{
     TCHAR* rbuf;
    rbuf = f_gets((TCHAR*)fatbuf, size, file);
    if(*rbuf == 0)
        return  ;//没有数据读到
    else
    {
        printf("\r\nThe String Readed Is:%s\r\n", rbuf);  	  
    }			    	
}

//需要_USE_STRFUNC>=1
//写一个字符到文件
//c:要写入的字符
//返回值:执行结果
u8 mf_putc(u8 c)
{
    return f_putc((TCHAR)c, file);
}

//写字符串到文件
//c:要写入的字符串
//返回值:写入的字符串长度
u8 mf_puts(u8*c)
{
    return f_puts((TCHAR*)c, file);
}

void fs_op_test_framework(void)
{
    int ret;
    fs_op_test_item_t *p_item = &fatfs_test_manager.start_test;
    LOG_I("into");

    while (p_item <= &fatfs_test_manager.end_test) {
        if (!p_item->op) {
            LOG_I("[%s] %s --> Not support", fatfs_test_manager.fs_name, p_item->name);
            ++p_item;
            continue;
        }
        ret = p_item->op();
        LOG_I("[%s] %s --> %s(%d)", fatfs_test_manager.fs_name, p_item->name, ret == 0 ? "SUCCESS" : "FAIL", ret);
        if (ret != 0) {
            // break;
            ++p_item;
            continue;   
        }
        ++p_item;
    }
}

/*********************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************/
// FATFS/其他FS 测试用例
static int start_fs_op_test(void)
{
    return 0;
}

static int end_fs_op_test(void)
{
    return 0;
}

static int file_open_test(void)
{
    // 与标准库文件操作不太一样,需要外部传入一个对象,
    // 该结构体很大,最好不要直接在内存中分配,占用栈内存太多
    FIL *fp;
    FRESULT ret;
    char buf[20] = "abcde";
    int write_num;
    int read_num;
    // LOG_I("struct FIL size:%d", sizeof(FIL)); // 556

    fp = mymalloc(sizeof(FIL));
    if (!fp)
        return -1;
    
    ret = f_open(fp, "0:/test1.txt", FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
    if (ret != FR_OK) {
        myfree(fp);
        return -2;
    }

    ret = f_write(fp, buf, sizeof(buf), &write_num);
    if (ret != FR_OK || write_num != sizeof(buf)) {
        f_close(fp);
        myfree(fp);
        return -3;
    }
    // LOG_D("write_num:%d", write_num);

    ret = f_lseek(fp, 0);
    if (ret != FR_OK) {
        f_close(fp);
        myfree(fp);
        return -4;
    }

    memset(buf, 0, sizeof(buf));
    ret = f_read(fp, buf, sizeof(buf), &read_num);
    if (ret != FR_OK || read_num != sizeof(buf)) {
        LOG_E("read_num:%d ret:%d", read_num, ret);
        f_close(fp);
        myfree(fp);
        return -5;
    }
    // LOG_D("read:%s read_num", buf, read_num);

    f_close(fp);
    myfree(fp);

    return 0;

    // 可以不用 goto 就不要用, 增大程序调试难度
// FAIL:
//     f_close(fp);
//     free(fp);
    // return -1;
}

// f_sync() 强行将 FATFS 缓冲区的内容更新到 NV 中
// 不仅仅时 f_write() 这里仅使用 f_write() 举例, 例如:更新文件属性等接口后, 最好调用一下
// 但也不能太频繁, 影响磁盘写入效率
// 对应的 C 接口 fflush()
static int file_op_async_test(void)
{
    FIL *fp;
    FRESULT ret;
    int write_num;
    int read_num;
    uint8_t buf[20] = "abcde"; // end with '\0'

    fp = mymalloc(sizeof(FIL));
    if (!fp)
        return -1;
    
    ret = f_open(fp, "0:/test1.txt", FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
    if (ret != FR_OK) {
        myfree(fp);
        return -2;
    }

    for (int i = 0; i < 50; ++i) {
        ret = f_write(fp, buf, strlen(buf) + 1, &write_num);
        if (ret != FR_OK || write_num != strlen(buf) + 1) {
            f_close(fp);
            myfree(fp);
            return -3;
        }
        // LOG_D("write[%d] %s", i, buf);
#if 0
        if (i == 10) {
            // 模拟程序异常退出
            *(unsigned char *)0 = 0; // 地址 0 处肯定无法直接访问
        }
#endif
        // 手动同步
#if DEBUG
        ret = f_sync(fp);
        if (ret != FR_OK)
            return -4;
#endif
    }

    f_close(fp); // 关闭文件的时候也会同步一次
    myfree(fp);

    return 0;
} 

// TODO:
// _USE_STRFUNC 可以配置每行 '\n' 或者 "\r\n" 结尾转换
// '\n' -> '\r\n'
// '\r\n' -> '\n'
static int file_op_string_input_test(void)
{
    FIL *fp;
    FRESULT ret;
    int write_num;
    int read_num;
    uint8_t buf[20];
    TCHAR *p_str;
    long offset;
    int i = 0;

    fp = mymalloc(sizeof(FIL));
    if (!fp)
        return -1;
    
    ret = f_open(fp, "0:/test1.txt", FA_READ | FA_WRITE);
    if (ret != FR_OK) {
        myfree(fp);
        return -2;
    }

    while (1) {
        memset(buf, 0x00, sizeof(buf));
        //NOTE: f_gets() 每次都会优先把 buf 读满, 而不是读到 '\0' 就结束
        p_str = f_gets(buf, 20, fp);
        if (!p_str) { // f_gets() 读到文件结束后返回空
            break;
        }
        // LOG_D("read_str[%d]:%s", i++, p_str);
        // LOG_HEX("buf", buf, sizeof(buf));
    }

    // 该版本不支持 F_APPEND 打开
    offset = f_tell(fp); // 当前文件总大小
    ret = f_lseek(fp, offset);
    if (ret != FR_OK) {
        f_close(fp);
        myfree(fp);
        return -3;
    }

    // 写入
    for (int i = 0; i < 10; ++i) {
        f_puts("put string test", fp); // 不会自动加回车换行, C 标准库的 fputs() 会自动换行
        f_putc('o', fp);
        f_putc('k', fp);
        f_puts("\r\n", fp);
    }

    f_close(fp);
    myfree(fp);

    return 0;
}

static int file_op_locate_test(void)
{
    FIL *fp;
    FRESULT ret;
    int write_num;
    int read_num;
    uint8_t buf[20];
    long size;

    fp = mymalloc(sizeof(FIL));
    if (!fp)
        return -1;
    
    ret = f_open(fp, "0:/test1.txt", FA_READ | FA_WRITE);
    if (ret != FR_OK) {
        myfree(fp);
        return -2;
    }

    // 调整偏移地址到末尾
    size = f_size(fp);
    f_lseek(fp, size);
    f_puts("This is the end", fp);

    // 文件空洞
    // 文件大小 100 byte, 直接调整到第 200 bytes (文件大小自动增大到 200)开始写入
    // LOG_D("original file size:%d", f_size(fp));
    f_lseek(fp, f_size(fp) + 100);
    // LOG_D("new file size:%d", f_size(fp));
    f_puts("Add a hole at the end of file", fp);

    f_close(fp);
    myfree(fp);

    return 0;
}

static void format_print_file_info(const FILINFO *fno)
{
    char out_str[256];

    snprintf(out_str, sizeof(out_str), "\t%c%c%c%c\t%s\t\t%d\t%d-%d-%d %d:%d:%d",
            fno->fattrib & AM_DIR ? 'd' : '-',
            fno->fattrib & AM_RDO ? 'r' : '-',
            fno->fattrib & AM_HID ? 'h' : '-',
            fno->fattrib & AM_SYS ? 's' : '-',
            fno->fname, fno->fsize,
            (fno->fdate >> 9) + 1980,
            (fno->fdate >> 5) & 0xF,
            (fno->fdate >> 0 & 0x1F),
            (fno->ftime >> 11) & 0x1F,
             (fno->ftime >> 5) & 0x3F,
              ((fno->ftime >> 0) & 0x1F) * 2);
    printf("%s\n", out_str);

    return;
}

// 获取文件大小,属性等内容
static int file_op_get_file_info_test(void)
{
    FRESULT ret;
    FILINFO file_info;
    char out_str[256];
    // TODO:// crash
    // 1 - 驱动器好像没有这个问题
    // ret = f_stat((const TCHAR *)"0:/example", &file_info);
    // if (ret != FR_OK) {
    //     return ret;
    // }
    // format_print_file_info(&file_info);

    return 0;
}

static int file_op_file_end_test(void)
{
    FIL *fp;
    FRESULT ret;
    int write_num;
    int read_num;
    uint8_t buf[20];
    long size;

    fp = mymalloc(sizeof(FIL));
    if (!fp)
        return -1;
    
    ret = f_open(fp, "0:/test1.txt", FA_READ | FA_WRITE);
    if (ret != FR_OK) {
        myfree(fp);
        return -2;
    }

    while (!f_eof(fp)) { // 是否到达文件尾部
        TCHAR * p_str = f_gets(buf, sizeof(buf), fp);
        if (p_str) {
            // LOG_D("read:%s", p_str);
        }
    }

    // Unix API:
    // https://linux.die.net/man/2/truncate
    // extern int truncate(const char *path, off_t length);
    // extern int ftruncate(int fd, off_t length);

    // FATFS api:从文件游标当前位置截断
    f_lseek(fp, 100);
    ret = f_truncate(fp);
    if (ret != FR_OK) {
        f_close(fp);
        myfree(fp);
        return -3;
    }

    // f_rewind(fp);
    f_lseek(fp, 0);
    while (!f_eof(fp)) { // 是否到达文件尾部
        TCHAR * p_str = f_gets(buf, sizeof(buf), fp);
        if (p_str) {
            // LOG_D("read after truncate:%s", p_str);
            ;
        }
    }
    
    f_close(fp);
    myfree(fp);

    return 0;
}

// 为文件分配连续的存储空间:
// (文件存储单位:簇(cluster),使用链表进行连接)
// 当对文件读写性能要求很高时可以使用(1.连续存储,2. SD卡驱动支持连续读取多个扇区)
static int file_op_allocate_sequential_space_test(void)
{
    FIL *fp;
    FRESULT ret;
    int write_num;
    int read_num;
    uint8_t buf[20];
    long size;

    fp = mymalloc(sizeof(FIL));
    if (!fp)
        return -1;
    
    ret = f_open(fp, "0:/test1.txt", FA_READ | FA_WRITE);
    if (ret != FR_OK) {
        myfree(fp);
        return -2;
    }

    //TODO:
    // 1.expand 之前文件大小必须为 0
    // 2. 最好保证最终文件大小不超过扩充的值, 否则后续再写可能导致最终的文件不连续, 出现异常
    // 3. 当前 FATFS 版本不支持该操作 f_expand()
    // ret = f_expand(&fp, 100 * 1024, 1); // 文件大小扩充为 100K
    // if (ret != FR_OK) {
    //     f_close(fp);
    //     myfree(fp);
    //     return -3;
    // }

    //需要了解 FATFS 内部存储原理
    //使用 disk_write 函数, 连续写多个扇区
    //根据文件的起始簇编号获取到文件的起始扇区号,以及需要写的扇区数量

    f_close(fp);
    myfree(fp);

    return 0;
}

// f_forward() 读取转发
// 一般情况下,数据传输流程:文件 -> 缓存 -> 其他硬件(串口, 网络等)
// 使用读取转发时,数据传输流程:文件 -> 其他硬件(串口, 网络等)
// 需要打开 #if _USE_FORWARD && _FS_TINY 才可以使用
// 
/**
 * @brief FATFS 会在 f_forward() 内部频繁调用 forward_fun() 知道传输结束
 * 
 * @param p p==NULL, 返回当前设备是否就绪, 1 就绪/0未就绪; p!= NULL, 需要发送的数据
 * @param len 
 * @return UINT 传输成功的字节数
 */
static UINT forward_fun(const BYTE *p, UINT len)
{
    if (!p) {
        // 这里仅测试使用, 直接返回就绪
        return 1;
    }
    // 这里使用串口打印作为测试
    
    uart1_send((const u8 *)p, len);

    // 仅测试使用, 直接返回长度
    return len;
}

static int file_op_read_transfer_test(void)
{
    FIL *fp;
    FRESULT ret;
    UINT write_num, read_num, transfer_num;
    uint8_t buf[20];
    long size;

    fp = mymalloc(sizeof(FIL));
    if (!fp)
        return -1;
    
    ret = f_open(fp, "0:/test1.txt", FA_READ);
    if (ret != FR_OK) {
        myfree(fp);
        return -2;
    }

    // 缺点:并不是每次都对 512 bytes, 不是固定的
    // ret = f_forward(fp, forward_fun, 4000, &transfer_num);
    // if (ret != FR_OK) {
    //     f_close(fp);
    //     myfree(fp);
    //     return -3;
    // }
    // LOG_D("transfer byte:%d", transfer_num);

    f_close(fp);
    myfree(fp);

    return 0;
}

/**
 * @brief 绝对路径
 * 
 * @param path 
 * @return int 
 */
static int f_scandir(const TCHAR *path)
{
    DIR dir;
    FRESULT ret;
    FILINFO file_info;

    f_chdir("0:");

    ret = f_opendir(&dir, path);
    if (ret != FR_OK) {
        return -1;
    }

    printf("%s\n", path);
    while (1) {
        ret = f_readdir(&dir, &file_info);
        if (ret != FR_OK) // read dir err
            break;
        if (file_info.fname[0] == '\0') // dir tail
            break;
        format_print_file_info(&file_info);
    }

    // TODO: 
    // 查找文件，当前版本不支持
    // f_findfirst(), f_findnext()

    f_closedir(&dir);
}

// UNIX:opendir(), closedir(), scandir(), seekdir(), telldir(), rewindir()...
static int dir_op_open_test(void)
{
    f_scandir("0:");

    return 0;
}

/**
 * @brief 创建随机文件, 仅用于测试
 * 
 */
static int create_random_file(const TCHAR *path, int file_size)
{
    FRESULT ret;
    FIL *fp;
    int write_num;
    char buff[10];

    fp = mymalloc(sizeof(FIL));
    if (!fp)
        return -1;

    ret = f_open(fp, path, FA_WRITE | FA_CREATE_ALWAYS);
    if (ret != FR_OK)
        return -2;

    for (int i = 0; i < file_size/5; ++i) {
        snprintf(buff, sizeof(buff), "%d", i);
        f_puts(buff, fp);
    }

    f_close(fp);

    return 0;
}

// f_mkdir() 不支持递归创建
static int dir_create_test(void)
{
    FRESULT ret;
    DIR dir;

    ret = f_mkdir("0:/example");
    if (ret != FR_OK)
        return -1;

    create_random_file("0:/example/test1.txt", 1025);
    create_random_file("0:/example/test2.txt", 500);
    create_random_file("0:/example/test3.txt", 100);

    ret = f_mkdir("0:/example/test");
    if (ret != FR_OK)
        return -1;
    create_random_file("0:/example/test/test1.txt", 100);
    create_random_file("0:/example/test/test2.txt", 200);
    create_random_file("0:/example/test/test3.txt", 300);

    LOG_I("after create:");
    f_scandir("0:/example");

    return 0;
}

// TODO:
// 嵌入式内部最好不要用递归, 改为链表
/**
 * @brief 递归删除文件夹/目录
 *        unlink() 只能删除空目录
 * 
 * @param path 
 * @return FRESULT 
 */
static FRESULT f_removedir(const TCHAR *path)
{
    DIR dir;
    FRESULT ret;
    FILINFO file_info;

    ret = f_chdir(path);
    if (ret != FR_OK) {
        return -1;
    }

    // ret = f_opendir(&dir, path);
    // 可能当前版本的 FATFS 存在 bug ???
    // TODO: 移植较新版本的该文件
    ret = f_opendir(&dir, "."); // f_chdir() 后打开工作目录可以直接使用 "."
    if (ret != FR_OK) {
        return -1;
    }

    while (1) {
        ret = f_readdir(&dir, &file_info);
        if (ret != FR_OK)
            break;
        if (file_info.fname[0] == '\0')
            break;
        if (file_info.fattrib & AM_DIR) { // 递归删除目录
            f_removedir(file_info.fname);
        } else {
            // file_info.fname 仅保存了文件名, 删除时需要能够找到文件(传入绝对路径/chdir())
            ret = f_unlink(file_info.fname); // 删除文件
            if (ret != FR_OK) {
                return -2;
            }
        }
    }

    f_closedir(&dir);

    // 切换到上一级目录, 删除该目录
    ret = f_chdir("..");
    if (ret != FR_OK) {
        return -3;
    }

    ret = f_unlink(path);
    if (ret != FR_OK) {
        return -4;
    }

    return 0;
}

static int dir_change_curr_test(void)
{
    DIR dir;
    FRESULT ret;
    FILINFO file_info;

    // f_chdir("0:/example");

    // 这里仍然需要加 0: 需要指定驱动器
    // ret = f_stat("0:example", &file_info); // "0:test/test.c"
    // if (ret != FR_OK) {
    //     return -1;
    // }
    // format_print_file_info(&file_info);

    // UNIX:remove()/unlink()
    // 删除文件
    ret = f_unlink("0:/test1.txt");
    if (ret != FR_OK) {
        return -2;
    }

    // 删除目录:
    // f_unlink() 不支持删除非空目录
    // 删除原理:先递归删除内部文件, 然后删除空文件夹
    // ret = f_removedir("0:/example");
    // if (ret != FR_OK)
    //     return -3;

    LOG_I("after delete:");
    f_scandir("0:/example");

    return 0;
}

static int dir_modify_attr_test(void)
{
    FRESULT ret;
    FILINFO file_info;

    // ret = f_chdir("0:/example");
    // if (ret != FR_OK)
    //     return -1;
    
    // ret = f_rename("0:/example/test1.txt", "0:/example/test1_1.txt");
    // if (ret != FR_OK)
    //     return -2;

    LOG_I("after rename:");
    f_scandir("0:/example");

    // f_rename() 同样可以用于文件/目录移动
    // ret = f_rename("0:/example/test2", "0:/example/test2_2");
    // if (ret != FR_OK)
    //     return -3;
    // f_scandir("0:/example");

    file_info.fdate = 0;
    file_info.ftime = 0;
    ret = f_utime("0:example", &file_info);
    if (ret != FR_OK)
        return -4;
    f_scandir("0:");

    // ret = f_chmod("0:/example/test1.txt", AM_RDO, AM_RDO | AM_HID);
    // if (ret != FR_OK)
    //     return -5;
    f_scandir("0:/example");

    // 设置默认驱动器(盘符 0:/1:/...)
    // 当使用多个驱动器挂载时, 需要指定盘符, 否则使用默认盘符
    // 设置默认驱动器: f_chdrive
    ret = f_chdrive("0:");
    if (ret != FR_OK)
        return -6;

    return 0;
}
