/**
 * @file sdcard.c
 * @author your name (you@domain.com)
 * @brief SD card
 * @version 0.1
 * @date 2022-12-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "sdcard.h"
#include "spi_stm.h" // 使用 SPI1 通讯
#include "usart.h"
#include <math.h>
#include "malloc.h"


// 可以使用 USB 转接器扩展为 USB 接口
// SPI1 与其他外设共用, 通过片选来控制总线
// 这里要注意的是SPI1被3个外设共用了：SD卡、W25Q64和NRF24L01，
// 在使用SD卡的时候，必须禁止其他外设的片选，以防干扰。
// 硬件:
// SD_CS - PA3
// SPI1 SCK - PA5
// SPI1 MISO - PA6
// SPI1 MOSI - PA7

//TODO:
// 1.sdcard 内部实际上有文件系统的, 要不然则呢么做到插到电脑上就可以显示的呢;
// 2. 热插拔如何实现;



/*********************************************************************************
 * MACRO
 *********************************************************************************/
#define MULTI_WRITE_START_TOKEN             0xFC //
#define MULTI_WRITE_STOP_TOKEN              0xFD // Stop Tran Token
#define SINGLE_WR_MULTI_READ_START_TOKEN    0xFE // start token, 令牌
// SD卡指令表,写的是 10 进制, 但是协议中写的是 16 进制
// 参考 《SD卡协议 2.0》, 分为 SD mode 通讯和 SPI mode 通讯两种方式
// 这里使用 SPI 方式，参考ch7 SPI mode
#define CMD0        0 //卡复位, reset
#define CMD1        1 // SEND_OP_COND
#define CMD8        8 //命令8 ，SEND_IF_COND
#define CMD9        9 //命令9 ，读CSD数据
#define CMD10       10 //命令10，读CID数据
#define CMD12       12 //命令12，停止数据传输, STOP_ TRANSMISSION
#define CMD13       13 //SEND_STATUS: in order to get the cause of the write problem
#define CMD16       16 //命令16，设置SectorSize(block size) 应返回0x00, SET_BLOCKLEN
#define CMD17       17 //命令17，读sector,  single block read
#define CMD18       18 //命令18，读Multi sector, Multiple Block read
#define CMD22       22 // get the number of well written write blocks when write error happen
#define CMD23       23 //命令23，设置多sector写入前预先擦除N个block
#define CMD24       24 //命令24，写sector
#define CMD25       25 //命令25，写Multi sector
#define CMD38       38 // ERASE
#define CMD41       41 //命令41，应返回0x00, SD_SEND_OP_COND
#define CMD42       42 // LOCK_UNLOCK
#define CMD55       55 //命令55，应返回0x01
#define CMD58       58 //命令58，读OCR信息, READ_OCR
#define CMD59       59 //命令59，使能/禁止CRC，应返回0x00, CRC_ON_OFF
/*********************************************************************************
 * PROTOTYPE
 *********************************************************************************/
//TODO:
typedef struct sdcard { // 与 SDcard 有关的信息
    u8 already_init : 1;
    u8 ver;
} sdcard_manager_t;

#if 0
typedef struct {
    uint32_t :7;
    uint32_t lov_voltage_range : 1;
    uint32_t : 7;
    uint32_t voltage : 9; // 电压范围
    uint32_t : 6;
    uint32_t card_capacity_status : 1;
    uint32_t card_powerup_status : 1;
} OCR_t;

typedef struct {
    uint32_t
} CID_t;

// CSD register parse, 16 byte, SD version 1.0
// SPI 发送每个字节都是先发送高 bit, 然后发送低 bit. 
// MSB first 
// 因此收到的数据在数组中是这样的:{[120:127], [112:119],..., [0:7]}
// 只需要将 MSB 转成 LSB 后就可以使用结构体进行强转(本身是 4 byte 对齐的)
// 最好不要使用结构体,
typedef struct CSD_v1 {
    uint8_t not_used : 1;
    uint8_t crc : 7;

    uint8_t  : 2;
    uint8_t file_format : 2;
    uint8_t temporary_write_protect : 1;
    uint8_t permanent_write_protect : 1;
    uint8_t copy_flag : 1;
    uint8_t file_format_group : 1;

    uint8_t  : 5;
    uint8_t partial_blocks_for_write : 1;
    uint8_t max_write_data_block_len : 2;

    uint8_t max_write_data_block_len_1 : 2;
    uint8_t write_speed_factor : 3;
    uint8_t  : 2;
    uint8_t write_protect_group_enable : 1;

    uint8_t write_protect_group_size : 7;
    uint8_t erase_sector_size : 1;

    uint8_t erase_sector_size_1 : 6;
    uint8_t erase_single_block_enable : 1;
    uint8_t device_size_multiplier : 1;

    uint8_t device_size_multiplier_1 : 2;
    uint8_t write_current_max : 3;
    uint8_t write_current_min : 3;

    uint8_t read_current_max : 3;
    uint8_t read_current_min : 3;
    uint8_t device_size : 2;

    uint8_t device_size_1 : 8;

    uint8_t device_size_2 : 2;
    uint8_t  : 2;
    uint8_t dsr_implented : 1;
    uint8_t read_block_misalignment : 1;
    uint8_t write_block_misalignment : 1;
    uint8_t partial_blocks_for_raad_allow : 1;

    uint8_t max_read_data_block_len : 4;
    uint8_t card_command_class : 4;

    uint8_t card_command_class_1 : 8;

    uint8_t data_transfer_rate : 1;
    uint8_t data_transfer_rate_1 : 7;

    uint8_t NSAC : 1;
    uint8_t NSAC_1 : 7;

    uint8_t TAAC : 1;
    uint8_t TAAC_1 : 7;

    uint8_t  : 1;
    uint8_t  : 5;
    uint8_t CSD_STRUCTURE : 2;
} CSD_v1_t;

// CSD register parse, 16 byte, SD version 2.0
typedef struct CSD_v2 {
    uint8_t not_used : 1;
    uint8_t crc : 7;

    uint8_t  : 2;
    uint8_t file_format : 2;
    uint8_t temporary_write_protect : 1;
    uint8_t permanent_write_protect : 1;
    uint8_t copy_flag : 1;
    uint8_t file_format_group : 1;

    uint8_t  : 5;
    uint8_t partial_blocks_for_write : 1;
    uint8_t max_write_data_block_len : 2;

    uint8_t max_write_data_block_len_1 : 2;
    uint8_t write_speed_factor : 3;
    uint8_t  : 2;
    uint8_t write_protect_group_enable : 1;

    uint8_t write_protect_group_size : 7;
    uint8_t erase_sector_size : 1;

    uint8_t erase_sector_size_1 : 6;
    uint8_t erase_single_block_enable : 1;
    uint8_t  : 1;

    uint8_t C_SIZE : 8;
    uint8_t C_SIZE_1 : 8;
    uint8_t C_SIZE_2 : 6;
    uint8_t  : 2;

    uint8_t  : 4;
    uint8_t DSR_IMP : 1;
    uint8_t READ_BLK_MISLAIGN : 1;
    uint8_t WRITE_BLK_MISALIGN : 1;
    uint8_t READ_BL_PARTIAL : 1;

    uint8_t READ_BL_LEN : 4;
    uint8_t CCC : 4;

    uint8_t CCC_1 : 8;

    uint8_t TRAN_SPEED : 8;

    uint8_t NSAC : 8;

    uint8_t TAAC : 8;

    uint8_t  : 6;
    uint8_t CSD_STRUCTURE : 2;
} CSD_v2_t;
#endif 

// TODO:
typedef struct CID {
    int a;
} CID_t;
/*********************************************************************************
 * PUBLIC VARIABLES
 *********************************************************************************/
u8  SD_Type=0; // SD卡的类型 
static sdcard_manager_t sdcard_manager;
/*********************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************/

/*********************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************/
////////////////////////////////////移植修改区///////////////////////////////////
//移植时候的接口
//data:要写入的数据
//返回值:读到的数据
u8 SD_SPI_ReadWriteByte(u8 data)
{
    return SPI1_ReadWriteByte(data);
}

//SD卡初始化的时候,需要低速
void SD_SPI_SpeedLow(void)
{
    SPI1_SetSpeed(SPI_SPEED_LOW);//设置到低速模式	
}

//SD卡正常工作的时候,可以高速了
void SD_SPI_SpeedHigh(void)
{
    SPI1_SetSpeed(SPI_SPEED_HIGH);//设置到高速模式	
}

//SPI硬件层初始化
void SD_SPI_Init(void)
{
    // 设置硬件上与SD卡相关联的控制引脚输出
    // 禁止其他外设(NRF/W25Q64)对SD卡产生影响
    RCC->APB2ENR |= 1 << 2;     //PORTA时钟使能 
    GPIOA->CRL &= 0XFFF000FF; 	// clear bits, PA2-F_CS, PA3-SD_CS; PA4-NRF_CS
    GPIOA->CRL |= 0X00033300;	// 0011, PA2.3.4 推挽输出, 最大输出 50MHz	    
    GPIOA->ODR |= 0X7 << 2;    	// PA2.3.4上拉, 其他外设暂时挂起
    SPI1_Init();
    SD_CS = 1;
}

//取消选择,释放SPI总线
void SD_DisSelect(void)
{
    SD_CS = 1;
    //在每次传输的结尾还必须提供 8 个额外的时钟,以允许 SD 完成任何未完结的工作
    //对这些额外时钟的输入数据必须全为1, 即:0xFF
     SD_SPI_ReadWriteByte(0xff); //提供额外的8个时钟???????????????????????
}

//选择sd卡,并且等待卡准备OK
//返回值:0,成功;1,失败;
u8 SD_Select(void)
{
    SD_CS = 0;
    if (SD_WaitReady() == 0) //等待成功
        return 0;
    else { //等待失败
        SD_DisSelect();
        return 1;
    }
    // SD_DisSelect();
    // return 1;//等待失败
}

//等待卡准备好
//返回值:0,准备好了;其他,错误代码
u8 SD_WaitReady(void)
{
    u32 t = 0;
    do {
        // 主机发送 0xFF 从机返回 0xFF???????????????
        // SD_GetResponse(0xFF)
        if(SD_SPI_ReadWriteByte(0XFF) == 0XFF) //OK
            return 0;
        t++;		  	
    } while(t < 0XFFFFFF);//等待 

    return 1; // 其他,错误代码
}

//等待SD卡回应
//Response:要得到的回应值
//返回值:0,成功得到了该回应值
//    其他,得到回应值失败
u8 SD_GetResponse(u8 wanted_response)
{
    u16 Count = 0xFFFF;//等待次数	 

    while ((SD_SPI_ReadWriteByte(0XFF) != wanted_response) && (Count != 0)) //等待得到准确的回应  	
        Count--;
    if (Count == 0) //得到回应失败   
        return MSD_RESPONSE_FAILURE;
    else 
        return MSD_RESPONSE_SUCCESS;//正确回应
}

//从sd卡读取一个数据包的内容
//buf:数据缓存区
//len:要读取的数据长度.
//返回值:0,成功;其他,失败;	
u8 SD_RecvData(u8 *buf, u16 len)
{
    // 等待SD卡发回数据起始令牌0xFE
    if(SD_GetResponse(SINGLE_WR_MULTI_READ_START_TOKEN) != MSD_RESPONSE_SUCCESS)
        return 1;
    while(len--) {//开始接收数据
        *buf++ = SPI1_ReadWriteByte(0xFF);
    }
    // 下面是2个伪CRC（dummy CRC）
    // Q:为什么是 伪 CRC,实际没有开启?
    // A:CRC 功能没有开启, 但是仍然需要发送到 slave, slave 会自动忽略
    SD_SPI_ReadWriteByte(0xFF);
    SD_SPI_ReadWriteByte(0xFF);	

    return 0;//读取成功
}

//向sd卡写入一个 block 的内容 512字节
//block_buff:数据缓存区, 数据长度必须是 512
//cmd:指令
//返回值:0,成功;其他,失败;
u8 SD_SendBlock(u8 *block_buff, u8 cmd)
{
    u16 t;

    if(SD_WaitReady())
        return 1;//等待准备失效

    // send command
    SD_SPI_ReadWriteByte(cmd);

    // if (cmd != MULTI_WRITE_STOP_TOKEN) { //不是结束指令
        // send data
        for (t = 0; t < 512; t++) {
            SPI1_ReadWriteByte(block_buff[t]);//提高速度,减少函数传参时间
        }

        SD_SPI_ReadWriteByte(0xFF);//忽略crc, CRC 功能关闭
        SD_SPI_ReadWriteByte(0xFF);

        t = SD_SPI_ReadWriteByte(0xFF);// 发送完一个 block 后, 接收响应
        // Data Response Token, bit[3:1], ‘010’ - Data accepted.
        //0x05 : 0000 0101
        if((t & 0x1F) != 0x05)
            return 2;//响应错误
    // }

    return 0;//写入成功
}

/**
 * @brief 向SD卡发送一个命令, SD 卡命令标准格式
 * 
 * @param cmd 命令, 1byte, 0~64(2^6)
 * @param arg 命令参数, 4 byte, MSB first
 * @param crc crc校验值, 1byte
 * @return u8 SD卡返回的响应
 */
u8 SD_SendCmd(u8 cmd, u32 arg, u8 crc)
{
    u8 r1;	
    u8 Retry = 0; 

    if (cmd > 64) // 0~64(2^6)
        return 0xFF;

    SD_DisSelect(); //取消上次片选
    if(SD_Select() != MSD_RESPONSE_SUCCESS) //片选失败
        return 0XFF;

    SD_SPI_ReadWriteByte(cmd | 0x40); //分别写入命令, SD 卡的命令格式:bit[7:6] 必须为 01 
    SD_SPI_ReadWriteByte(arg >> 24); // MSB first
    SD_SPI_ReadWriteByte(arg >> 16);
    SD_SPI_ReadWriteByte(arg >> 8);
    SD_SPI_ReadWriteByte(arg);	  
    SD_SPI_ReadWriteByte(crc); 

    if (cmd == CMD12) // 停止数据传输, 额外提供几个时钟
        SD_SPI_ReadWriteByte(0xff); //Skip a stuff byte when stop reading

    //等待响应，或超时退出
    //Unlike the SD Memory Card protocol,
    //in the SPI mode, the card will always respond to a command. 
    Retry = 0X1F;
    do {
        r1 = SD_SPI_ReadWriteByte(0xFF);
        // LOG_D("CMD:%d r1:%X", cmd, r1);
    } while((r1 & 0X80) && (Retry-- != 0)); // 没有数据时返回 0xFF,等待 bit[7] != 1 ？？？？？？？？
    
    //返回状态值
    return r1;
}

//获取SD卡的CID信息，包括制造商信息
//输入: u8 *cid_data(存放CID的内存，至少16Byte）	  
//返回值:0：NO_ERR
//		 1：错误														   
u8 SD_GetCID(u8 *cid_data)
{
    u8 r1;
 
    //发CMD10命令，读CID
    r1 = SD_SendCmd(CMD10, 0, 0x01);
    if (r1 == 0x00) {
        r1 = SD_RecvData(cid_data, 16);//接收16个字节的数据	 
    }
    SD_DisSelect();//取消片选

    if(r1 != MSD_RESPONSE_SUCCESS) // 读取失败
        return 1;

    return 0; // 读取成功
}
     
/**
 * @brief 获取 CSD 寄存器的值
 * 		  获取SD卡的CSD信息，包括容量和速度信息
 * 
 * @param csd_data 存放CID的内存，至少16Byte）
 * @return u8 0：NO_ERR; 1：错误
 */
u8 SD_GetCSD(u8 *csd_data)
{
    u8 r1;	 
    r1 = SD_SendCmd(CMD9, 0, 0x01);//发CMD9命令，读CSD
    if (r1 == MSD_RESPONSE_SUCCESS) {
        // Q:先接收高字节????
        //
        r1 = SD_RecvData(csd_data, 16); //接收16个字节的数据 
    }
    SD_DisSelect();//取消片选
    if(r1 != MSD_RESPONSE_SUCCESS) // 读取失败
        return 1;
    // else 
    
    return 0; // 读取成功
}


/**
 * @brief 获取SD卡的总扇区数（扇区数）
 * 	      每扇区的字节数必为512，因为如果不是512，则初始化不能通过.		
 * 
 * @return * u32 0:取容量出错; 成功:SD卡的容量, unit:扇区数(总容量/512字节)
 */
u32 SD_GetSectorCount(void)
{
    // csd[0]: bit[7:0]
    // csd[1]: bit[15:8]
    // csd[2]: bit[23:16]
    // csd[idx] : bit[(idx+1)*8-1 : idx*8]
    u8 csd[16] = {0};
    u32 csize;
    u32 Capacity = 0;  
    u8 n;
    u8 i;
    u8 device_size_multipler;
    u16 BLOCKNR = 0;
    u16 MULT = 0;
    u16 BLOCK_LEN = 0;
    u32 sector;
    u8 csd_structure = 0xFF;
    u32 write_block_len, read_block_len;

    // error:取CSD信息，如果期间出错，返回0
    if(SD_GetCSD(csd) != MSD_RESPONSE_SUCCESS) {
        LOG_E("get CSD failed");
        return 0;
    }
    reverse_byte(csd, 16); // MSB first 先接收高字节进行逆序处理

    csd_structure = csd[15] >> 6; // csd[127:126]， 11000000b
    // 如果为SDHC卡，按照下面方式计算
    // if((csd[0] & 0xC0) == 0x40){ //V2.00的卡, csd[0] & 1100 0000 == 0100 0000
    // 	LOG_I("sdcard version 2.0\r\n");
    // 	csize = csd[9] + ((u16)csd[8] << 8) + 1; // big endian?
    // 	Capacity = (u32)csize << 10;//得到扇区数	 		   
    // }
    // else{ //V1.XX的卡
    // 	LOG_I("sdcard version 1.0\r\n");
    // 	n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
    // 	csize = (csd[8] >> 6) + ((u16)csd[7] << 2) + ((u16)(csd[6] & 3) << 10) + 1;
    // 	Capacity= (u32)csize << (n - 9); //得到扇区数   
    // }

    if (csd_structure == 0x00) { // SD version 1.0
        // LOG_I("%d %d %d\r\n", pCSD->device_size, pCSD->device_size_1, pCSD->device_size_2);
        // csize = pCSD->device_size | (pCSD->device_size_1 << 2) | (pCSD->device_size_2 << 10);
        // device_size_multipler = pCSD->device_size_multiplier | (pCSD->device_size_multiplier_1 << 1);
        // MULT = (int)pow(2, device_size_multipler + 2);
        // LOG_I("C_SIZE:%d, device_multipler:%d, read data block len:%d\r\n", csize, device_size_multipler, pCSD->max_read_data_block_len);
        // LOG_I("MULT = %d\r\n", MULT);
        // LOG_I("BLOCKNR = %d\r\n", (csize + 1) * MULT);
        // LOG_I("BLOCK_LEN = %d\r\n", (int)(pow(2, pCSD->max_read_data_block_len)));
        // LOG_I("memory capacity = %d\r\n", BLOCKNR  * BLOCK_LEN);
    } else if (csd_structure == 0x01) { // SD version 2.0
        read_block_len = pow(2, csd[10] & 0x0F); // // 参考 SD card2.0 手册, READ_BL_LEN
        csize = csd[6] | (csd[7] << 8) | ((csd[8] & 0x3F) << 16); // csd[69:48]
        write_block_len = ((csd[2] & 0xC0) >> 6) | ((csd[3] & 0x03) << 2);
        write_block_len = pow(2, write_block_len); // 参考 SD card2.0 手册, WRITE_BL_LEN
        Capacity = (csize + 1) * 512; // K byte
    } else {
        LOG_E("not support ver:%X", csd_structure);
    }
    sector = Capacity  * 1024 / 512; // 1 扇区 512 byte

    return sector;
}

u32 SD_PrintBaseInfo(void)
{
    // csd[0]: bit[7:0]
    // csd[1]: bit[15:8]
    // csd[2]: bit[23:16]
    // csd[idx] : bit[(idx+1)*8-1 : idx*8]
    u8 csd[16] = {0};
    u32 csize;
    u32 Capacity = 0;  
    u8 n;
    u8 i;
    u8 device_size_multipler;
    u16 BLOCKNR = 0;
    u16 MULT = 0;
    u16 BLOCK_LEN = 0;
    u32 sector;
    u8 csd_structure = 0xFF;
    u32 write_block_len, read_block_len;

    // error:取CSD信息，如果期间出错，返回0
    if(SD_GetCSD(csd) != MSD_RESPONSE_SUCCESS) {
        LOG_E("get CSD failed");
        return 0;
    }
    reverse_byte(csd, 16); // MSB first 先接收高字节进行逆序处理

    // LOG_I("sdcard CSD: sizeof(CSD_T):%d", sizeof(CSD_v2_t));
    LOG_HEX("CSD:", csd, 16);
    csd_structure = csd[15] >> 6; // csd[127:126]， 11000000b
    LOG_I("csd structure:%#02X", csd[15] >> 6);
    // 如果为SDHC卡，按照下面方式计算
    // if((csd[0] & 0xC0) == 0x40){ //V2.00的卡, csd[0] & 1100 0000 == 0100 0000
    // 	LOG_I("sdcard version 2.0\r\n");
    // 	csize = csd[9] + ((u16)csd[8] << 8) + 1; // big endian?
    // 	Capacity = (u32)csize << 10;//得到扇区数	 		   
    // }
    // else{ //V1.XX的卡
    // 	LOG_I("sdcard version 1.0\r\n");
    // 	n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
    // 	csize = (csd[8] >> 6) + ((u16)csd[7] << 2) + ((u16)(csd[6] & 3) << 10) + 1;
    // 	Capacity= (u32)csize << (n - 9); //得到扇区数   
    // }

    if (csd_structure == 0x00) { // SD version 1.0
        LOG_I("sdcard ver1.0");
        // LOG_I("%d %d %d\r\n", pCSD->device_size, pCSD->device_size_1, pCSD->device_size_2);
        // csize = pCSD->device_size | (pCSD->device_size_1 << 2) | (pCSD->device_size_2 << 10);
        // device_size_multipler = pCSD->device_size_multiplier | (pCSD->device_size_multiplier_1 << 1);
        // MULT = (int)pow(2, device_size_multipler + 2);
        // LOG_I("C_SIZE:%d, device_multipler:%d, read data block len:%d\r\n", csize, device_size_multipler, pCSD->max_read_data_block_len);
        // LOG_I("MULT = %d\r\n", MULT);
        // LOG_I("BLOCKNR = %d\r\n", (csize + 1) * MULT);
        // LOG_I("BLOCK_LEN = %d\r\n", (int)(pow(2, pCSD->max_read_data_block_len)));
        // LOG_I("memory capacity = %d\r\n", BLOCKNR  * BLOCK_LEN);
    } else if (csd_structure == 0x01) { // SD version 2.0
        LOG_I("TTAC:%#X", csd[14]); // csd[119:112]
        LOG_I("NSAC:%#X", csd[13]); // csd[111:104]
        LOG_I("TRAN_SPEED:%#X", csd[12]); // csd[103:96]
        LOG_I("CCC:%#X", csd[10] & 0xF0 | csd[11] << 8); // csd[95:84]
        read_block_len = pow(2, csd[10] & 0x0F); // // 参考 SD card2.0 手册, READ_BL_LEN
        LOG_I("READ_BL_LEN:%d Bytes", read_block_len); // csd[83:80]
        LOG_I("READ_BL_PARTIAL:%d", csd[9] & 0x80); // [79:79]
        LOG_I("WRITE_BLK_MISALIGN:%d", csd[9] & 0x40); // [78:78]
        LOG_I("READ_BLK_MISALIGN:%d", csd[9] & 0x20); // [77:77]
        LOG_I("DSR_IMP:%d", csd[9] & 0x10); // [76:76]
        csize = csd[6] | (csd[7] << 8) | ((csd[8] & 0x3F) << 16); // csd[69:48]
        LOG_I("CSIZE:%d", csize); // [69:48]
        LOG_I("ERASE_BLK_EN:%d", (csd[5] & 0x40) ? true : false); //[46:46]
        LOG_I("SECTOR_SIZE:%#X", csd[4] & 0x80 | (csd[5] & 0x3F >> 1)); //[45:39]
        LOG_I("WRITE_PROTECT_GROUP_SIZE:%#X", csd[4] & 0x7F); // [38:32]
        LOG_I("WP_PROTECT_GRP_ENABLE:%d", (csd[3] & 0x80) >> 7); // [31:31]
        LOG_I("WRITE_SPEED_FACTOR:%d", (csd[3] & 0x13) >> 2); // [28:26]
        write_block_len = ((csd[2] & 0xC0) >> 6) | ((csd[3] & 0x03) << 2);
        write_block_len = pow(2, write_block_len); // 参考 SD card2.0 手册, WRITE_BL_LEN
        LOG_I("WRITE_BL_LEN:%d Bytes", write_block_len); //[25:22]
        LOG_I("WRITE_BL_PARTIAL:%d", csd[2] & 0x20 ? true : false); // [21:21]
        LOG_I("note used:%d", csd[0] & 0x01);
        Capacity = (csize + 1) * 512; // K byte
        LOG_I("sdcard capacity:%d KByte %dMByte %dGByte", Capacity, 
                Capacity / 1024, Capacity/1024/1024);
    } else {
        LOG_E("not support ver:%X", csd_structure);
    }
    sector = Capacity  * 1024 / 512; // 1 扇区 512 byte
    LOG_I("sector:%d\r\n", sector);

    return sector;
}

/**
 * @brief 初始化SD卡
 * 		  按照 SD 卡提供的标准初始化流程进行
 * 
 * @return u8 0:初始化成功
 */
u8 SD_Initialize(void)
{
    u8 r1; // 存放SD卡的返回值
    u16 retry; // 用来进行超时计数
    u8 buf[4];  
    u16 i;

    if (sdcard_manager.already_init) {
        LOG_E("sdcard already init");
        return 0;
    }

    LOG_I("sdcard init");
    SD_SPI_Init(); //初始化IO
    SD_SPI_SpeedLow(); //设置到低速模式, 小于 400KHz
    for(i = 0; i < 10; i++) //等待最少74个脉冲
        SD_SPI_ReadWriteByte(0XFF);

    retry = 20;

    do {
        // The SD Card is powered up in the SD mode. 
        // It will enter SPI mode if the CS signal is asserted 
        // (negative) during the reception of the reset command (CMD0).
        // 0x40, 0x0, 0x0, 0x0, 0x0, 0x95
        r1 = SD_SendCmd(CMD0, 0x00000000, 0x95); //复位 SD 卡, 并设置进入 SPI 模式进入IDLE状态
    } while ((r1 != 0X01) && retry--); // 发送复位命令 CMD0, 等待返回 0x01, bit[0] (idle state)为高电平

    SD_Type = 0;//默认无卡
    if (r1 == 0X01) {
        // 发送 CMD8, 检查是否支持 SD2.0
        // The structure of the first (MSB) byte is identical to response type R1. The 
        // other four bytes contain the card operating voltage information and echo back of check pattern in 
        // argument and are specified by the same definition as R7 response in SD mode.
        if(SD_SendCmd(CMD8, 0x000001AA, 0x87) == 0x01) //SD V2.0
        {
            for(i = 0; i < 4; i++)
                buf[i] = SD_SPI_ReadWriteByte(0XFF); //Get trailing return value of R7 resp
            LOG_HEX("CMD8 Rsp buffer:", buf, 4);
            LOG_I("command version:%d", (buf[0] >> 4)); // buf[31:28]
            LOG_I("voltage accepted:%d", buf[2] & 0x04); // buf[11:8]
            LOG_I("check pattern:%X", buf[3]); // echo back

            if(buf[2] == 0X01 && buf[3] == 0XAA)//卡是否支持2.7~3.6V
            {
                retry=0XFFFE;
                do
                {
                    SD_SendCmd(CMD55, 0, 0X01);	//发送CMD55, 告诉 SD 卡, 下一个是特定应用命令
                    r1 = SD_SendCmd(CMD41, 0x40000000, 0X01); //发送CMD41, 发送给主机容量支持信息和激活卡初始化流程
                }while(r1 && retry--);

                // 读取 OCR 寄存器(操作条件寄存器)
                if(retry && (SD_SendCmd(CMD58, 0, 0X01) == 0))//鉴别SD2.0卡版本开始
                {
                    for(i = 0; i < 4; i++) //得到OCR值
                        buf[i] = SD_SPI_ReadWriteByte(0XFF);
                    LOG_HEX("CMD58 rsp buffer", buf, 4);
                    if(buf[0] & 0x40) // 检查CCS
                        SD_Type = SD_TYPE_V2HC;    
                    else 
                        SD_Type = SD_TYPE_V2;   
                }
            }
        } else {//SD V1.x/ MMC	V3
            SD_SendCmd(CMD55, 0, 0X01); //发送CMD55, 告诉 SD 卡, 下一个是特定应用命令
            r1 = SD_SendCmd(CMD41, 0, 0X01); //发送CMD41, 发送给主机容量支持信息和激活卡初始化流程
            if (r1 <= 1) {
                SD_Type = SD_TYPE_V1;
                retry = 0XFFFE;
                //等待退出IDLE模式
                do 
                {
                    SD_SendCmd(CMD55, 0, 0X01);	//发送CMD55
                    r1=SD_SendCmd(CMD41, 0, 0X01); //发送CMD41
                }while(r1 && retry--);
            } else { //MMC 卡不支持 CMD55+CMD41 识别
                SD_Type = SD_TYPE_MMC;//MMC V3
                retry = 0XFFFE;
                //等待退出IDLE模式
                do 
                {
                    r1=SD_SendCmd(CMD1, 0, 0X01);//发送CMD1
                }while(r1 && retry--);  
            }
            if(retry == 0 || (SD_SendCmd(CMD16, 512, 0X01) != 0))
                SD_Type = SD_TYPE_ERR;//错误的卡
        }
    }
    SD_DisSelect();//取消片选
    SD_SPI_SpeedHigh();//高速
    LOG_I("SD card type:%d", SD_Type);
    if(SD_Type != 0) {// success
#if 1 // 初始化成功后打印 SD 卡基础信息
        SD_PrintBaseInfo();
#endif
        sdcard_manager.already_init = 1;
        return 0;
    }
    else if(r1)
        return r1;

    return 0xaa;//其他错误
}

/**
 * @brief 读 SD 卡
 * 
 * @param[out] buf 数据缓存区, 要求大于 512 bytes
 * @param[in]  sector 扇区索引, 第几个扇区
 * @param[in]  cnt 要读取的扇区个数
 * @return u8 0,ok; 其他,失败
 */
u8 SD_ReadDisk(u8 *buf, u32 sector, u8 cnt)
{
    u8 r1;

    if(SD_Type != SD_TYPE_V2HC)
        sector <<= 9; //转换为字节地址, (1 << 9) == 512, sector *= 512

    if (cnt == 1) { // 读取一个扇区
        r1 = SD_SendCmd(CMD17, sector, 0X01); //读命令:CMD17, 读取一个块的数据
        if(r1 == MSD_RESPONSE_SUCCESS) {//指令发送成功
            r1 = SD_RecvData(buf, 512);// 接收512个字节	   
        }
    } else { // 读取 cnt 个扇区
        r1 = SD_SendCmd(CMD18, sector, 0X01);// CMD18 连续读块命令
        do
        {
            r1 = SD_RecvData(buf, 512); // 接收512个字节	 
            buf += 512;  
        }while(--cnt && r1 == MSD_RESPONSE_SUCCESS);
        // In case of a Multiple Block Read the stop transmission is performed 
        // using STOP_TRAN Command (CMD12).
        SD_SendCmd(CMD12, 0, 0X01);	// CMD12发送停止命令
    }
    SD_DisSelect();//取消片选

    return r1;//
}

/**
 * @brief 写 SD 卡
 * 
 * @param[in]  buf 数据缓存区
 * @param[in]  sector 起始扇区
 * @param[in]  cnt 扇区数, 用于决定使用 Single Write 还是 Muliple Write
 * @return u8 0,ok;其他,失败.
 */
u8 SD_WriteDisk(u8*buf, u32 sector, u8 cnt)
{
    u8 r1;
    if(SD_Type != SD_TYPE_V2HC)
        sector *= 512;//转换为字节地址

    if(cnt == 1){ // 写入一个扇区
        r1 = SD_SendCmd(CMD24, sector, 0X01);//读命令:CMD24 写入一个块的数据
        if(r1 == MSD_RESPONSE_SUCCESS)//指令发送成功
        {
            // Q:为什么先发送 0xFE?
            // A:single write start token, 主机先发送 start_token:0xFE, 详见 SDcard 协议
            r1 = SD_SendBlock(buf, SINGLE_WR_MULTI_READ_START_TOKEN);//写512个字节	   
        }
    }
    else{
        if(SD_Type != SD_TYPE_MMC){
            SD_SendCmd(CMD55, 0, 0X01);	// CMD55, 告诉 SD 卡, 下一个是特定应用命令
            SD_SendCmd(CMD23, cnt, 0X01);// 发送指令 CMD23 
        }
        r1 = SD_SendCmd(CMD25, sector, 0X01); // 连续写命令:CMD25, 连续读取 block
        if(r1 == MSD_RESPONSE_SUCCESS){
            do
            {
                // Q:为什么先发送 0xFC?
                // A:《SD card protocol V2.0》 multi write start token
                r1 = SD_SendBlock(buf, MULTI_WRITE_START_TOKEN);//接收512个字节	 
                buf += 512;  
            }while(--cnt && (r1 == MSD_RESPONSE_SUCCESS));

            r1 = SD_WaitReady();
            if (r1 != 0) // wait ready failed
                return r1;

            // stop trans token
            // r1 = SD_SendBlock(0, MULTI_WRITE_STOP_TOKEN);//接收512个字节 
            SD_SPI_ReadWriteByte(MULTI_WRITE_STOP_TOKEN);
        }
    }   
    SD_DisSelect();//取消片选

    return r1;
}

