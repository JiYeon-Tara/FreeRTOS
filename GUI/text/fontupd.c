#include "fontupd.h"
#include "ff.h"	  
#include "w25q64.h"
#include "ILI93xx.h"
#include "malloc.h"

/*********************************************************************************
 * MACRO
 *********************************************************************************/
//字库存放起始地址
// W25Q64 flash 分区:
// 前面 0 ~ 4.8M 空间给 FATFS 使用, 
// 4.8M - 4.9M(4.8M+100K) 给用户使用
// 4.9M 以后用于存放字库, 字库占用 3.09MB
//字库在 flash 中的存储方式:
// font_info_header + UNICODE2GBK + GBK12 + GBK16 + GBK24
#define FONTINFOADDR 	(4916 + 100) * 1024 //MiniSTM32是从4.8M+100K地址开始的
/*********************************************************************************
 * PRIVATE VARIABLES
 *********************************************************************************/
//字库信息结构体. 
//用来保存字库基本信息，地址，大小等
_font_info ftinfo;

// NOTE:
// GBK 编码字库的生成方法,《点阵字库生成软件》设置
// 1.936 GBK
// 2. 宋体, 12
// 3. w*h:12 * 12
// 4.DZK
// 5. 纵向:从上到下, 从左至右
// 注意要正确显示还需要调整 .c 的编码格式, 

//字库存放在sd卡中的路径(手动拷贝到 SD 卡)
const u8 *GBK24_PATH = "0:/SYSTEM/FONT/GBK24.BIN"; //GBK24的存放位置
const u8 *GBK16_PATH = "0:/SYSTEM/FONT/GBK16.BIN"; //GBK16的存放位置
const u8 *GBK12_PATH = "0:/SYSTEM/FONT/GBK12.BIN"; //GBK12的存放位置
const u8 *UNIGBK_PATH = "0:/SYSTEM/FONT/UNIGBK.BIN"; //UNIGBK.BIN的存放位置


/**
 * @brief 显示当前字体更新进度
 * 
 * @param fsize 字库文件大小
 * @param pos 当前文件指针位置
 * @return u32 
 */
u32 fupd_prog(u32 fsize, u32 pos)
{
    float prog;
    u8 t = 0XFF;
    
    prog = (float)pos / fsize;
    prog *= 100;
    
    if (t != prog) {
        // LCD_ShowString(x+3*size/2,y,240,320,size,"%");		
        t = prog;
        if (t > 100)
            t = 100;
        // LCD_ShowNum(x,y,t,3,size);//显示数值
        LOG_I("update font progress:%d", t);
    }

    return 0;					    
}

// 从文件系统中读出, 写入到 flash 中
//fxpath:字库在文件系统中的保存路径
//fx:更新的内容 0,ungbk;1,gbk12;2,gbk16;3,gbk24;
//返回值:0,成功;其他,失败.
u8 updata_fontx(u8 *fxpath, u8 fx)
{
    u32 flashaddr=0;								    
    FIL * fftemp = NULL;
    u8 *tempbuf = NULL;
     u8 res;	
    u16 bread;
    u32 offx=0;
    u8 rval=0;

    fftemp = (FIL*)mymalloc(sizeof(FIL));	//分配内存
    if (fftemp == NULL)
        rval = 1;
    tempbuf = mymalloc(4096);	//分配4096个字节空间
    if (tempbuf == NULL) {
        rval = 1;
    }
     res = f_open(fftemp, (const TCHAR*)fxpath, FA_READ); 
     if(res) {
        rval = 2;//打开文件失败
    }

     if (rval != 0) {
        LOG_E("error rval:%d", rval);
        free(tempbuf);
        free(fftemp);
        return -1;
    }

    switch (fx) {
        case FONT_UNICODE2GBK: //更新UNIGBK.BIN
            // TODO:这样写有 bug, 必须按照顺序更新
            ftinfo.ugbkaddr = FONTINFOADDR + sizeof(ftinfo); //信息头之后，紧跟UNIGBK转换码表
            ftinfo.ugbksize = fftemp->fsize; //UNIGBK大小
            flashaddr = ftinfo.ugbkaddr;
            break;
        case FONT_GBK12:
            // TODO:这样写有 bug, 必须按照顺序更新
            ftinfo.f12addr = ftinfo.ugbkaddr + ftinfo.ugbksize; //UNIGBK之后，紧跟GBK12字库
            ftinfo.gbk12size = fftemp->fsize; //GBK12字库大小
            flashaddr = ftinfo.f12addr; //GBK12的起始地址
            break;
        case FONT_GBK16:
            ftinfo.f16addr = ftinfo.f12addr + ftinfo.gbk12size;	//GBK12之后，紧跟GBK16字库
            ftinfo.gbk16size = fftemp->fsize; //GBK16字库大小
            flashaddr = ftinfo.f16addr; //GBK16的起始地址
            break;
        case FONT_GBK24:
            ftinfo.f24addr = ftinfo.f16addr+ftinfo.gbk16size; //GBK16之后，紧跟GBK24字库
            ftinfo.gkb24size = fftemp->fsize; //GBK24字库大小
            flashaddr = ftinfo.f24addr; //GBK24的起始地址
            break;
    }

    while (res == FR_OK) {//死循环执行
        res = f_read(fftemp, tempbuf, 4096, (UINT *)&bread); //读取数据	 
        if(res != FR_OK)
            break; //执行错误
        //TOOD:
        // 这里可能会越界
        // SPI_Flash_Write(tempbuf, offx + flashaddr, bread); //从0开始写入4096个数据  
        SPI_Flash_Write(tempbuf, offx + flashaddr, 4096); //从0开始写入4096个数据
        offx += bread;
        fupd_prog(fftemp->fsize, offx); //进度显示
        if (bread != 4096)
            break; //读完了
    }

    f_close(fftemp);
    myfree(fftemp);	//释放内存
    myfree(tempbuf); //释放内存
    LOG_I("update %s success", fxpath);

    return res;
}

//更新字体文件,UNIGBK,GBK12,GBK16,GBK24一起更新
//x,y:提示信息的显示地址
//size:字体大小
//提示信息字体大小										  
//返回值:0,更新成功;
//		 其他,错误代码.	
u8 update_font(void)
{
    u8 res = 0XFF;

    ftinfo.fontok = 0XFF;
    SPI_Flash_Write((u8*)&ftinfo, FONTINFOADDR, sizeof(ftinfo));	//清除之前字库成功的标志.防止更新到一半重启,导致的字库部分数据丢失.
    SPI_Flash_Read((u8*)&ftinfo, FONTINFOADDR, sizeof(ftinfo));	//重新读出ftinfo结构体数据

    // LCD_ShowString(x,y,240,320,size,"Updating UNIGBK.BIN");		
    LOG_I("Updating UNIGBK.BIN");
    res = updata_fontx((u8*)UNIGBK_PATH, FONT_UNICODE2GBK); //更新UNIGBK.BIN
    if (res) {
        LOG_E("unicode2gbk update failed");
        return 1;
    }

    LOG_I("Updating GBK12.BIN");
    //  LCD_ShowString(x,y,240,320,size,"Updating GBK12.BIN  ");
    res = updata_fontx((u8*)GBK12_PATH, FONT_GBK12); //更新GBK12.FON
    if (res) {
        LOG_E("GBK12.BIN update failed");
        return 2;
    }

    LOG_I("Updating GBK16.BIN");
    // LCD_ShowString(x,y,240,320,size,"Updating GBK16.BIN  ");
    res = updata_fontx((u8*)GBK16_PATH, FONT_GBK16); //更新GBK16.FON
    if(res) {
        LOG_E("GBK16.BIN update failed");
        return 3;
    }

    LOG_I("Updating GBK24.BIN");
    // LCD_ShowString(x,y,240,320,size,"Updating GBK24.BIN  ");
    res = updata_fontx((u8*)GBK24_PATH, FONT_GBK24); //更新GBK24.FON
    if (res) {
        LOG_E("GBK24.BIN update failed");
        return 4;
    }

    //全部更新好了
    ftinfo.fontok = 0XAA;
    SPI_Flash_Write((u8*)&ftinfo, FONTINFOADDR, sizeof(ftinfo));	//保存字库信息
    LOG_I("update font success");

    return 0;//无错误.		 
}

//初始化字体
//返回值:0,字库完好.
//		 其他,字库丢失
u8 font_init(void)
{
    SPI_Flash_Init();  
    SPI_Flash_Read((u8*)&ftinfo, FONTINFOADDR, sizeof(ftinfo));//读出ftinfo结构体数据
    LOG_I("font_init");
#if 0
    LOG_I("font struct size:%d obj size:%d", sizeof(_font_info), sizeof(ftinfo));
#endif
    if (ftinfo.fontok != 0XAA) {
        LOG_E("invalid font flag %d", ftinfo.fontok);
        return 1;			//字库错误. 
    }

    return 0;		    
}





























