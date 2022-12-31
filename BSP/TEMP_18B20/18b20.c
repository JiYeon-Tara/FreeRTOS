/**
 * @file 18b20.c
 * @author your name (you@domain.com)
 * @brief temperature sensor DS18B20
 * @version 0.1
 * @date 2022-02-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "18B20.h"

//复位DS18B20
void DS18B20_Rst(void)	   
{                 
	DS18B20_IO_OUT(); //SET PA0 OUTPUT
    DS18B20_DQ_OUT = 0; //拉低DQ
    delay_us(750);    //拉低750us
    DS18B20_DQ_OUT = 1; // DQ=1, 上拉 
	delay_us(15);     //15US, 进入接收模式
}

/**
 * @brief 等待DS18B20的回应
 * 
 * @return u8 1:未检测到DS18B20的存在; 0:存在
 */
u8 DS18B20_Check(void) 	   
{   
	u8 retry = 0;
	DS18B20_IO_IN();//SET PA0 INPUT	 
    while (DS18B20_DQ_IN && retry < 200){
		retry++;
		delay_us(1);
	}
	if(retry >= 200)
        return 1;
	else 
        retry = 0;
    while (!DS18B20_DQ_IN && retry < 240){
		retry++;
		delay_us(1);
	}
	if(retry >= 240)
        return 1;	    
	return 0;
}

/**
 * @brief 从DS18B20读取一个位 bit
 * 
 * @return u8 1/0
 */
u8 DS18B20_Read_Bit(void) 			 // read one bit
{
    u8 data;
	DS18B20_IO_OUT();//SET PA0 OUTPUT
    DS18B20_DQ_OUT = 0; 
	delay_us(2);
    DS18B20_DQ_OUT = 1; 
	DS18B20_IO_IN();//SET PA0 INPUT
	delay_us(12);
	if(DS18B20_DQ_IN)
        data = 1;
    else 
        data=0;	 
    delay_us(50);     
      
    return data;
}

/**
 * @brief 写入一个 bit
 * 
 * @param bit 
 */
void DS18B20_Write_Bit(u8 bit)
{

}

/**
 * @brief 从DS18B20读取一个字节
 * 
 * @return u8 读到的数据
 */
u8 DS18B20_Read_Byte(void)    // read one byte
{        
    u8 i, j, dat = 0;
    dat = 0;
	for (i = 1; i <= 8; i++) {
        j = DS18B20_Read_Bit();
        dat = (j << 7) | (dat >> 1); // 先读取 LSB
    }						    
    return dat;
}

/**
 * @brief 写一个字节到DS18B20
 * 
 * @param dat 要写入的字节
 */
void DS18B20_Write_Byte(u8 dat)     
{             
    u8 j;
    u8 testb;
	DS18B20_IO_OUT();//SET PA0 OUTPUT;
    for (j = 1; j <= 8; j++) 
	{
        testb = dat & 0x01; // 从低字节开始写入
        dat = dat >> 1;
        if (testb){ // 写 1 时序
            DS18B20_DQ_OUT = 0;// Write 1
            delay_us(2);                            
            DS18B20_DQ_OUT = 1;
            delay_us(60);             
        }
        else{ // 写 0 时序
            DS18B20_DQ_OUT = 0;// Write 0
            delay_us(60);             
            DS18B20_DQ_OUT = 1;
            delay_us(2);                          
        }
    }

    return;
}

//开始温度转换
void DS18B20_Start(void)// ds1820 start convert
{
    DS18B20_Rst();	   
	DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0x44);// convert
} 

/**
 * @brief 初始化DS18B20的IO口 DQ 同时检测DS的存在
 * 
 * @return * u8 1:不存在; 0:存在
 */
u8 DS18B20_Init(void)
{
	RCC->APB2ENR |= 1 << 2; //使能PORTA口时钟 
	GPIOA->CRL &= 0XFFFFFFF0;//PORTA0 推挽输出
	GPIOA->CRL |= 0X00000003;
	GPIOA->ODR |= 1<<0;      //输出1
	DS18B20_Rst();
	return DS18B20_Check();
}  

/**
 * @brief 从ds18b20得到温度值
 *        精度：0.1C
 * 
 * @return * short 温度值 （-550~1250）
 */
short DS18B20_Get_Temp(void)
{
    u8 tempFlag;
    u8 TL, TH;
	short tem;
    DS18B20_Start ();                    // ds1820 start convert
    DS18B20_Rst();
    DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom 发送 SKIP ROM 命令（0XCC）
    DS18B20_Write_Byte(0xbe);// convert	发读存储器命令（0XBE）   
    TL = DS18B20_Read_Byte(); // LSB   
    TH = DS18B20_Read_Byte(); // MSB  
	    	  
    if(TH > 7){ // ??????????????????????????????????????????????
        TH = ~TH;
        TL = ~TL; 
        tempFlag = 0;//温度为负  
    }
    else 
        tempFlag = 1;//温度为正	

    tem = TH; //获得高八位
    tem <<= 8;    
    tem += TL;//获得底八位
    tem = (float)tem * 0.625;//转换     
	if(tempFlag)
        return tem; //返回温度值
	else 
        return -tem; // ???????????????????????????????????????
}