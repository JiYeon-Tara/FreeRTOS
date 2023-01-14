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
// 硬件:
// SD_CS - PA3
// SPI1 SCK - PA5
// SPI1 MISO - PA6
// SPI1 MOSI - PA7
					   
u8  SD_Type=0; // SD卡的类型 


/********************
 * MACRO
 ********************/


// CSD register parse, 16 byte, SD version 1.0
typedef struct CSD_v1 {
	uint8_t not_used : 1;
	uint8_t crc : 7;

	uint8_t reserved1 : 2;
	uint8_t file_format : 2;
	uint8_t temporary_write_protect : 1;
	uint8_t permanent_write_protect : 1;
	uint8_t copy_flag : 1;
	uint8_t file_format_group : 1;

	uint8_t resvered2 : 5;
	uint8_t partial_blocks_for_write : 1;
	uint8_t max_write_data_block_len : 2;

	uint8_t max_write_data_block_len_1 : 2;
	uint8_t write_speed_factor : 3;
	uint8_t reserved3 : 2;
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
	uint8_t resvered4 : 2;
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

	uint8_t resvered5 : 1;
	uint8_t resvered5_1 : 5;
	uint8_t CSD_STRUCTURE : 2;
} CSD_v1_t;

// CSD register parse, 16 byte, SD version 2.0
typedef struct CSD_v2 {
	uint8_t not_used : 1;
	uint8_t crc : 7;

	uint8_t reserved1 : 2;
	uint8_t file_format : 2;
	uint8_t temporary_write_protect : 1;
	uint8_t permanent_write_protect : 1;
	uint8_t copy_flag : 1;
	uint8_t file_format_group : 1;

	uint8_t resvered2 : 5;
	uint8_t partial_blocks_for_write : 1;
	uint8_t max_write_data_block_len : 2;

	uint8_t max_write_data_block_len_1 : 2;
	uint8_t write_speed_factor : 3;
	uint8_t reserved3 : 2;
	uint8_t write_protect_group_enable : 1;

	uint8_t write_protect_group_size : 7;
	uint8_t erase_sector_size : 1;

	uint8_t erase_sector_size_1 : 6;
	uint8_t erase_single_block_enable : 1;
	uint8_t reserved4 : 1;

	uint8_t C_SIZE : 8;

	uint8_t C_SIZE_1 : 8;

	uint8_t C_SIZE_2 : 6;
	uint8_t reserved5 : 2;

	uint8_t reserved5_1 : 4;
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

	uint8_t reserved : 6;
	uint8_t CSD_STRUCTURE : 2;
} CSD_t;


/********************
 * FUNCTION
 ********************/
static uint8_t reverse_array(uint8_t *arr, uint16_t len);


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
 	SPI1_SetSpeed(SPI_SPEED_256);//设置到低速模式	
}

//SD卡正常工作的时候,可以高速了
void SD_SPI_SpeedHigh(void)
{
 	SPI1_SetSpeed(SPI_SPEED_2);//设置到高速模式	
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
 	SD_SPI_ReadWriteByte(0xff); //提供额外的8个时钟???????????????????????
}

//选择sd卡,并且等待卡准备OK
//返回值:0,成功;1,失败;
u8 SD_Select(void)
{
	SD_CS = 0;
	if(SD_WaitReady() == 0) //等待成功
        return 0;
	else{ //等待失败
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
	do
	{
		// 主机发送 0xFF 从机返回 0xFF???????????????
		// SD_GetResponse(0xFF)
		if(SD_SPI_ReadWriteByte(0XFF) == 0XFF) //OK
            return 0;
		t++;		  	
	}while(t < 0XFFFFFF);//等待 
	return 1; // 其他,错误代码
}

//等待SD卡回应
//Response:要得到的回应值
//返回值:0,成功得到了该回应值
//    其他,得到回应值失败
u8 SD_GetResponse(u8 Response)
{
	u16 Count = 0xFFFF;//等待次数	 
  						  
	while ((SD_SPI_ReadWriteByte(0XFF) != Response) && (Count != 0)) //等待得到准确的回应  	
        Count--;  
	if (Count == 0) //得到回应失败   
        return MSD_RESPONSE_FAILURE;
	else 
        return MSD_RESPONSE_NO_ERROR;//正确回应
}

//从sd卡读取一个数据包的内容
//buf:数据缓存区
//len:要读取的数据长度.
//返回值:0,成功;其他,失败;	
u8 SD_RecvData(u8 *buf, u16 len)
{
	// 主机发送 0xFF 从机回复 0xFE ???????????????????????????????
	if(SD_GetResponse(0xFE) != MSD_RESPONSE_NO_ERROR) // 等待SD卡发回数据起始令牌0xFE
        return 1;
    while(len--)//开始接收数据
    {
        *buf = SPI1_ReadWriteByte(0xFF);
        buf++;
    }
    // 下面是2个伪CRC（dummy CRC）
    SD_SPI_ReadWriteByte(0xFF);
    SD_SPI_ReadWriteByte(0xFF);	

    return 0;//读取成功
}

//向sd卡写入一个数据包的内容 512字节
//buf:数据缓存区
//cmd:指令
//返回值:0,成功;其他,失败;	
u8 SD_SendBlock(u8 *buf, u8 cmd)
{
	u16 t;

	if(SD_WaitReady())
        return 1;//等待准备失效

	// send command
	SD_SPI_ReadWriteByte(cmd);

	if(cmd != 0XFD)//不是结束指令
	{
		// send data
		for(t = 0; t < 512; t++){
            SPI1_ReadWriteByte(buf[t]);//提高速度,减少函数传参时间
		}

		// send CRC
	    SD_SPI_ReadWriteByte(0xFF);//忽略crc
	    SD_SPI_ReadWriteByte(0xFF);

		// receive response
		t = SD_SPI_ReadWriteByte(0xFF);//接收响应
		if((t & 0x1F) != 0x05) // 0x05 : 0000 0101
            return 2;//响应错误
	}

    return 0;//写入成功
}

//向SD卡发送一个命令
//输入: u8 cmd   命令 
//      u32 arg  命令参数
//      u8 crc   crc校验值	   
//返回值:SD卡返回的响应															  
u8 SD_SendCmd(u8 cmd, u32 arg, u8 crc)
{
    u8 r1;	
	u8 Retry = 0; 

	SD_DisSelect();//取消上次片选
	if(SD_Select() != MSD_RESPONSE_NO_ERROR) //片选失败
        return 0XFF;

	//发送
    SD_SPI_ReadWriteByte(cmd | 0x40); //分别写入命令, SD 卡的命令格式:bit[6] 必须为 1 
    SD_SPI_ReadWriteByte(arg >> 24);
    SD_SPI_ReadWriteByte(arg >> 16);
    SD_SPI_ReadWriteByte(arg >> 8);
    SD_SPI_ReadWriteByte(arg);	  
    SD_SPI_ReadWriteByte(crc); 

	if(cmd == CMD12) // 停止数据传输
        SD_SPI_ReadWriteByte(0xff); //Skip a stuff byte when stop reading

    //等待响应，或超时退出
	Retry = 0X1F;
	do
	{
		r1 = SD_SPI_ReadWriteByte(0xFF);
	}while((r1 & 0X80) && (Retry-- != 0)); // 等待 bit[7] != 1 
	
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
    if(r1 == 0x00){
		r1 = SD_RecvData(cid_data, 16);//接收16个字节的数据	 
    }
	SD_DisSelect();//取消片选

	if(r1 != MSD_RESPONSE_NO_ERROR) // 读取失败
        return 1;
	// else

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
    if(r1 == MSD_RESPONSE_NO_ERROR)
	{
		// 先接收高字节????
    	r1 = SD_RecvData(csd_data, 16); //接收16个字节的数据 
    }
	SD_DisSelect();//取消片选
	if(r1 != MSD_RESPONSE_NO_ERROR) // 读取失败
        return 1;
	// else 
    
	return 0; // 读取成功
}  


/**
 * @brief 获取SD卡的总扇区数（扇区数）
 * 	      每扇区的字节数必为512，因为如果不是512，则初始化不能通过.		
 * 
 * @return * u32 0:取容量出错; 其他:SD卡的容量(扇区数/512字节)
 */
u32 SD_GetSectorCount(void)
{
    u8 csd[16] = {0};
    u32 Capacity = 0;  
    u8 n;
	u32 csize; // device size, bit[73:62]
	u8 i;
	CSD_t *pCSD = NULL;
	u8 device_size_multipler;
	u16 BLOCKNR = 0;
	u16 MULT = 0;
	u16 BLOCK_LEN = 0;
	u32 sector;
					    
	// error:取CSD信息，如果期间出错，返回0
    if(SD_GetCSD(csd) != MSD_RESPONSE_NO_ERROR) 
        return 0;
	
	// 先接收高字节进行逆序处理
	reverse_array(csd, 16);

	pCSD = (CSD_t*)&csd[0];

	if(pCSD->not_used != 1){
		printf("error, bit[0]:%d\r\n", pCSD->not_used);
		return 0;
	}

    // 如果为SDHC卡，按照下面方式计算
    // if((csd[0] & 0xC0) == 0x40){ //V2.00的卡, csd[0] & 1100 0000 == 0100 0000
	// 	printf("sdcard version 2.0\r\n");
	// 	csize = csd[9] + ((u16)csd[8] << 8) + 1; // big endian?
	// 	Capacity = (u32)csize << 10;//得到扇区数	 		   
    // }
    // else{ //V1.XX的卡
	// 	printf("sdcard version 1.0\r\n");
	// 	n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
	// 	csize = (csd[8] >> 6) + ((u16)csd[7] << 2) + ((u16)(csd[6] & 3) << 10) + 1;
	// 	Capacity= (u32)csize << (n - 9); //得到扇区数   
    // }

	// print debug information
	printf("sdcard CSD: sizeof(CSD_T):%d\r\n", sizeof(CSD_t));
	for(i = 0; i < 16; ++i){
		printf("%02X ", csd[i]);
	}
	printf("\r\n");

	// SD version 1.0
	// printf("%d %d %d\r\n", pCSD->device_size, pCSD->device_size_1, pCSD->device_size_2);
	// csize = pCSD->device_size | (pCSD->device_size_1 << 2) | (pCSD->device_size_2 << 10);
	// device_size_multipler = pCSD->device_size_multiplier | (pCSD->device_size_multiplier_1 << 1);
	// MULT = (int)pow(2, device_size_multipler + 2);
	// printf("C_SIZE:%d, device_multipler:%d, read data block len:%d\r\n", csize, device_size_multipler, pCSD->max_read_data_block_len);
	// printf("MULT = %d\r\n", MULT);
	// printf("BLOCKNR = %d\r\n", (csize + 1) * MULT);
	// printf("BLOCK_LEN = %d\r\n", (int)(pow(2, pCSD->max_read_data_block_len)));
	// printf("memory capacity = %d\r\n", BLOCKNR  * BLOCK_LEN);

	// SD version 2.0
	// printf("%02X %02X %02X\r\n", pCSD->C_SIZE, pCSD->C_SIZE_1, pCSD->C_SIZE_2);
	csize = pCSD->C_SIZE | (pCSD->C_SIZE_1 << 8) | (pCSD->C_SIZE_2 << 16);
	Capacity = (csize + 1) * 512; // K byte
	// printf("C_SIZE:%d\r\n", csize);
	printf("sdcard capacity:%d Kbyte, %dMbyte\r\n", Capacity, Capacity / 1024);

	sector = Capacity  * 1024 / 512; // 1 扇区 512 byte
	printf("sector:%d\r\n", sector);
    // return Capacity;

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
    u8 r1;      // 存放SD卡的返回值
    u16 retry;  // 用来进行超时计数
    u8 buf[4];  
	u16 i;

    printf("sdcard init\r\n");
	SD_SPI_Init();		//初始化IO
 	SD_SPI_SpeedLow();	//设置到低速模式 
 	for(i = 0; i < 10; i++) //发送最少74个脉冲
        SD_SPI_ReadWriteByte(0XFF);
	retry = 20;

	// 发送复位命令 CMD0
	do
	{
		r1 = SD_SendCmd(CMD0, 0, 0x95); //复位 SD 卡, 进入IDLE状态
	}while((r1 != 0X01) && retry--); // 发送复位命令 CMD0, 等待返回 0x01
    
 	SD_Type = 0;//默认无卡
	if(r1 == 0X01)
	{
		// 发送 CMD8, 检查是否支持 SD2.0
		if(SD_SendCmd(CMD8, 0x1AA, 0x87) == 1) //SD V2.0
		{
			for(i = 0; i < 4; i++)
				buf[i] = SD_SPI_ReadWriteByte(0XFF);	//Get trailing return value of R7 resp
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
					if(buf[0] & 0x40) // 检查CCS
						SD_Type = SD_TYPE_V2HC;    
					else 
						SD_Type = SD_TYPE_V2;   
				}
			}
		}
		else {//SD V1.x/ MMC	V3
			SD_SendCmd(CMD55, 0, 0X01); //发送CMD55, 告诉 SD 卡, 下一个是特定应用命令
			r1 = SD_SendCmd(CMD41, 0, 0X01);	//发送CMD41, 发送给主机容量支持信息和激活卡初始化流程
			if(r1 <= 1)
			{
				SD_Type = SD_TYPE_V1;
				retry = 0XFFFE;
				//等待退出IDLE模式
				do 
				{
					SD_SendCmd(CMD55, 0, 0X01);	//发送CMD55
					r1=SD_SendCmd(CMD41, 0, 0X01); //发送CMD41
				}while(r1 && retry--);
			}
			else{ //MMC 卡不支持 CMD55+CMD41 识别
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
	printf("SD card type:%d\r\n", SD_Type);
	if(SD_Type != 0)
		return 0;
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

	if(cnt == 1) // 读取一个扇区
	{
		r1 = SD_SendCmd(CMD17, sector, 0X01); //读命令:CMD17, 读取一个块的数据
		if(r1 == MSD_RESPONSE_NO_ERROR) {//指令发送成功
			r1 = SD_RecvData(buf, 512);// 接收512个字节	   
		}
	}
	else{ // 读取 cnt 个扇区
		r1 = SD_SendCmd(CMD18, sector, 0X01);// CMD18 连续读块命令
		do
		{
			r1 = SD_RecvData(buf, 512); // 接收512个字节	 
			buf += 512;  
		}while(--cnt && r1 == MSD_RESPONSE_NO_ERROR); 	
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
 * @param[in]  cnt 扇区数
 * @return u8 0,ok;其他,失败.
 */
u8 SD_WriteDisk(u8*buf,u32 sector,u8 cnt)
{
	u8 r1;
	if(SD_Type != SD_TYPE_V2HC)
		sector *= 512;//转换为字节地址

	if(cnt == 1){ // 吸入一个扇区
		r1 = SD_SendCmd(CMD24, sector, 0X01);//读命令:CMD24 写入一个块的数据
		if(r1 == MSD_RESPONSE_NO_ERROR)//指令发送成功
		{
			// 为什么先发送 0xFE???????????????
			r1 = SD_SendBlock(buf, 0xFE);//写512个字节	   
		}
	}
	else{
		if(SD_Type != SD_TYPE_MMC){
			SD_SendCmd(CMD55, 0, 0X01);	// CMD55, 告诉 SD 卡, 下一个是特定应用命令
			SD_SendCmd(CMD23, cnt, 0X01);// 发送指令 CMD23 
		}
 		r1 = SD_SendCmd(CMD25, sector, 0X01); // 连续读命令:CMD25, 连续读取 block
		if(r1 == MSD_RESPONSE_NO_ERROR){
			do
			{
				// 为什么先发送 0xFC???????????????
				r1 = SD_SendBlock(buf, 0xFC);//接收512个字节	 
				buf += 512;  
			}while(--cnt && (r1 == MSD_RESPONSE_NO_ERROR));

			// ??????????????????????????
			// buffer == NULL???????????
			r1 = SD_SendBlock(0, 0xFD);//接收512个字节 
		}
	}   
	SD_DisSelect();//取消片选

	return r1;
}	


static uint8_t reverse_array(uint8_t *arr, uint16_t len)
{
	uint8_t *temp = (uint8_t*)mymalloc(sizeof(uint8_t) * len);
    uint16_t ix;
	if(!temp){
		printf("malloc error\r\n");
		return -1;
	}

	mymemcpy(temp, (uint8_t*)arr, len);

	for(ix = 0; ix < len; ++ix){
		arr[ix] = temp[len - 1 - ix];
	}

	myfree(temp);

	return 0;
}

