#include "ds18b20.h"
#include "usart.h"

//复位DS18B20
void DS18B20_Reset(void)	   
{                 
	DS18B20_IO_OUT(); //SET PA0 OUTPUT
    DS18B20_DQ_OUT = 0; //拉低 DQ
    delay_us(750);    //拉低750us
    DS18B20_DQ_OUT = 1; //DQ=1, 释放总线 
	delay_us(15);     //15US
}


/**
 * @brief 等待DS18B20的回应
 * 
 * @return u8 返回0:存在; 返回1:未检测到DS18B20的存在
 */
u8 DS18B20_Check(void) 	   
{   
	u8 retry = 0;
	DS18B20_IO_IN();//SET PA0 INPUT	 
    while (DS18B20_DQ_IN && (retry < 200)) // 等待 DS18B20 拉低总线产生应答信号
	{
		retry++;
		delay_us(1);
	}
	if(retry >= 200) // 60-240us, 没有应答信号
        return 1;
	else 
        retry = 0;

    // 18B20 拉低总线后等待总线被拉高
    while (!DS18B20_DQ_IN && retry < 240)
	{
		retry++;
		delay_us(1);
	};
	if(retry >= 240) // 没有拉高
        return 1;	    
	return 0;
}

/**
 * @brief 从DS18B20读取一个位
 * 
 * @return u8 返回值：1/0
 */
u8 DS18B20_Read_Bit(void) 			 // read one bit
{
    u8 data;
	DS18B20_IO_OUT();   //SET PA0 OUTPUT
    DS18B20_DQ_OUT = 0;  // 拉低 2us
	delay_us(2);

    DS18B20_DQ_OUT = 1; // 拉高并设置为输入开始读
	DS18B20_IO_IN();    //SET PA0 INPUT
	delay_us(12);

	if(DS18B20_DQ_IN)
        data = 1;
    else 
        data = 0;	 
    delay_us(50);
           
    return data;
}


/**
 * @brief 从DS18B20读取一个字节
 * 
 * @return u8 读到的数据
 */
u8 DS18B20_Read_Byte(void)    // read one byte
{        
    u8 i, j, data;
    data = 0;
	for (i = 1; i <= 8; i++) 
	{
        j = DS18B20_Read_Bit();
        data = (j << 7) | (data >> 1); // 先读低字节
        // data = data >> 1;
        // data |= j << 7;
    }
    return data;
}

/**
 * @brief 向 DS18B20 写入一个 bit
 * 
 * @param writeBit 0/1
 * @return u8 0 - success; 1 - fail
 */
u8 DS18B20_Write_Bit(u8 writeBit)
{
    DS18B20_IO_OUT();
    if(writeBit){
        DS18B20_DQ_OUT=0;// Write 1
        delay_us(2);                            
        DS18B20_DQ_OUT=1;
        delay_us(60);  
    }
    else{
        DS18B20_DQ_OUT=0;// Write 0
        delay_us(60);             
        DS18B20_DQ_OUT=1;
        delay_us(2);      
    }

    return 0;
}

//写一个字节到DS18B20
//dat：要写入的字节
void DS18B20_Write_Byte(u8 dat)     
 {
    u8 j;
    u8 testb;
	DS18B20_IO_OUT();//SET PA0 OUTPUT;
    for (j = 1; j <= 8; j++) 
	{
        testb = dat & 0x01;
        dat = dat >> 1;
        if (testb) // 写 1 时序:拉低总线 2us 然后拉高 60us
        {
            DS18B20_DQ_OUT = 0;// Write 1
            delay_us(2);                            
            DS18B20_DQ_OUT = 1;
            delay_us(60);             
        }
        else // 写 0 时序:拉低总线 60us 然后拉高 2us
        {
            DS18B20_DQ_OUT = 0;// Write 0
            delay_us(60);             
            DS18B20_DQ_OUT = 1;
            delay_us(2);                          
        }
    }
}

//开始温度转换
void DS18B20_Start(void)// ds1820 start convert
{
    DS18B20_Reset();	   
	DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0x44);// convert
} 


/**
 * @brief 初始化DS18B20的IO口 DQ 同时检测DS的存在
 * 
 * @return * u8 返回0:存在 返回1:不存在
 */
u8 DS18B20_Init(void)
{
	RCC->APB2ENR |= 1 << 2;    //使能PORTA口时钟 
	GPIOA->CRL &= 0XFFFFFFF0;//PORTA0 推挽输出
	GPIOA->CRL |= 0X00000003;
	GPIOA->ODR |= 1 << 0;      //输出1
	DS18B20_Reset();
	
    printf("DS18B20 init\n");
    return DS18B20_Check();
}  

//从ds18b20得到温度值
//精度：0.1C
//返回值：温度值 （-550~1250） 
short DS18B20_Get_Temp(void)
{
    u8 temp;
    u8 TL,TH;
	short tem;
    DS18B20_Start ();                    // ds1820 start convert
    DS18B20_Reset();
    DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0xbe);// convert	    
    TL = DS18B20_Read_Byte(); // LSB   
    TH = DS18B20_Read_Byte(); // MSB  
	    	  
    if(TH > 7)
    {
        TH = ~TH; // 按位取反
        TL = ~TL; 
        temp=0;//温度为负  
    }
    else 
        temp = 1;//温度为正	  	  
    tem = TH; //获得高八位
    tem <<= 8;    
    tem += TL;  //获得底八位
    tem = (float)tem * 0.625;//转换     
	if(temp)
        return tem; //返回温度值
	else 
        return -tem;    
}
