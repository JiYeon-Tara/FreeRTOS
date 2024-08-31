#include "delay.h"
#include "usart.h"
#include "rtc.h" 		    
#include <stdint.h>

#include "ulog.h"
       
_calendar_obj calendar;//时钟结构体 

/***********
 * MACRO
 ***********/
// 在后备区域保存 RTC 的时钟源类型, 优先外部低速, 然后内部低速, 初始化成功之后在 BKP->DR1 写标志
#define RTC_CLOCK_LSI		0X5050
#define RTC_CLOCK_LSE		0x5051
// 中断优先级
#define RTC_PRE_EMPTION_PRI			0
#define RTC_SUB_PRI					0
#define RTC_PRI_GROUP				2


/**
 * @brief set real time UTC time
 * 		  为了方便校准时间, 可以直接使用 AT 命令的方式; RTC_Set();
 * 
 */
void setRealTimeRtc()
{
    // 第一次初始化的时间, 直接这样会有 bug
    calendar.w_year = 2024;
    calendar.w_month = 7;
    calendar.w_date = 21;
    calendar.hour = 14;
    calendar.min = 44;
    calendar.sec = 0;
}

/**
 * @brief 实时时钟配置, 初始化RTC时钟,同时检测时钟是否工作正常
 * 		（2）使用 STM 的备份寄存器来保存是否被初始化过的标志;
 * 		BKP->DR1用于保存是否第一次配置的设置
 * 		后备寄存器区域, 即使 Vdd 电源切断, 也仍然由 Vbat 供电; 当系统在待机模式下被唤醒, 或者系统复位/电源复位,也不会被复位
 *
 * @return u8 返回0:正常; 其他:错误代码
 */
u8 RTC_Init(void)
{
    u8 temp=0;

    //检查是不是第一次配置时钟
    if(BKP->DR1 != RTC_CLOCK_LSE)//之前使用的不是LSE, 重新配置 RTC
    {
        RCC->APB1ENR |= 1 << 28; // power interface clock 使能电源时钟	    
        RCC->APB1ENR |= 1 << 27; // 使能备份时钟, BKP   
        PWR->CR |= 1 << 8; // 取消备份区域 & RTC 的写保护, 允许写
        RCC->BDCR |= 1 << 16; // 备份区域软复位	   
        RCC->BDCR &= ~(1 << 16); // 备份区域软复位结束	  	 
        RCC->BDCR |= 1 << 0; // 使能外部低速振荡器 LSE 32KHz, 32.768KHz
        while ((!(RCC->BDCR & 0X02)) && temp < 250) {/* 等待 LSE 就绪 */
            temp++;
            delay_ms(10);
        };
        
        if (temp >= 250) {// LSE 初始化时钟失败,外部晶振有问题, 使用内部低速晶振 LSI
            RCC->CSR |= 1 << 0; //开启内部低速振荡器  LSI
            while(!(RCC->CSR & (1<<1))) // 开启内部低速振荡器 LSI
                ;
            RCC->BDCR |= 2 << 8; // LSI作为RTC时钟 
            BKP->DR1 = RTC_CLOCK_LSI; //标记使用LSI作为RTC时钟
        } else {
            // 外部晶振 LSE 初始化成功
            RCC->BDCR |= 1 << 8; //LSE作为RTC时钟 
            BKP->DR1 = RTC_CLOCK_LSE; //标记使用LSE作为RTC时钟
        }
        RCC->BDCR |= 1 << 15;//RTC时钟使能	
        
        while(!(RTC->CRL & (1 << 5))) // bit[5], 等待RTC寄存器上一个操作完成	
            ; 
        while(!(RTC->CRL & (1 << 3))) // bit[3], 等待RTC寄存器同步，寄存器同步标志 (Registers synchronized flag) 置 1 后才可以对 RTC 进行后续操作
            ;

        /** 1.允许秒中断 **/
        RTC->CRH |= (1 << 0); //允许秒中断
        while(!(RTC->CRL & (1 << 5))) //等待RTC寄存器操作完成后, 才可以进行其他 RTC 操作
            ; 
        RTC->CRL |= 1 << 4; // 打开 RTC 配置, 允许对 RTC 寄存器进行配置	 

        /* 如果输入时钟频率是32.768kHz(fRTCCLK)，这个寄存器中写入7FFFh可获得周期为1秒钟的信号 */
        RTC->PRLH = 0X0000;
        RTC->PRLL = 0x7FFF; // 时钟周期设置(有待观察,看是否跑慢了?)理论值：32767

        /** 2.允许闹钟中断 **/
        RTC->CRH |= (1 << 1);
        while(!(RTC->CRL & (1 << 5)))
            ;

        /** 设置时间 **/
        // RTC_Set(2014,3,8,22,10,55);  // 第一次设置时间	
        RTC_Set(calendar.w_year, calendar.w_month, calendar.w_date, calendar.hour, calendar.min, calendar.sec);
        
        RTC->CRL &= ~(1 << 4); // 关闭 RTC 配置允许位, 配置更新， RTC_CRC->bit[4] = 0;
        while(!(RTC->CRL & (1 << 5)))
            ;   //等待RTC寄存器操作完成		 									  
        LOG_I("RTC fist initialization\n", );
    }
    else { // 系统继续计时, RTC 已经配置过
        while(!(RTC->CRL & (1 << 3))) //等待RTC寄存器同步  
            ;
        /** 1.允许秒中断 **/
        RTC->CRH |= (1 << 0); 
        while(!(RTC->CRL & (1 << 5))) //等待RTC寄存器操作完成
            ;

        /** 2.允许闹钟中断 **/
        // 如果需要设置闹钟的话需要设置 RTC_ALR 寄存器
        RTC->CRH |= (1 << 1);
        while(!(RTC->CRL & (1 << 5))) //等待RTC寄存器操作完成
            ;
        
        LOG_I("RTC OK, not reopen\n");
    }
    
    // 配置中断
    MY_NVIC_Init(RTC_PRE_EMPTION_PRI, RTC_SUB_PRI, RTC_IRQn, RTC_PRI_GROUP);//优先级设置  
  
    RTC_Get();//更新时间

    LOG_I("Time:%d/%d/%d %d:%d:%d  Week:%d\r\n", calendar.w_year, calendar.w_month,calendar.w_date, \
                                                   calendar.hour, calendar.min, calendar.sec, calendar.week);
    return 0;
}

/**
 * @brief RTC时钟中断
 *        允许秒中断时, 每秒触发一次, 
 * 
 */
void RTC_IRQHandler(void)
{
    if (RTC->CRL & (1 << 0)) { //秒钟中断
        RTC_Get(); //更新时间   
        // LOG_I("second interrupt Happend! [%d]\r\n",calendar.sec);
    }
    if(RTC->CRL & (1 << 1)) { //闹钟中断
        RTC->CRL &= ~(0x0002); //清闹钟中断	  
        LOG_I("Alarm Happened!\n");		   
    }
    RTC->CRL &= 0X0FFA; //清除溢出，秒钟中断标志
    while (!(RTC->CRL & (1<<5))); //等待RTC寄存器操作完成

    return; 	    						 	   	 
}

//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//year:年份
//返回值:该年份是不是闰年.1,是.0,不是
u8 Is_Leap_Year(u16 year)
{
    if(year%4==0) //必须能被4整除
    { 
        if(year % 100 == 0) 
        { 
            if(year % 400 == 0)
                return 1;//如果以00结尾,还要能被400整除 	   
            else 
                return 0;   
        }
        else 
            return 1;   
    }
    else 
        return 0;	
}

//设置时钟
//把输入的时钟转换为秒钟
//以1970年1月1日为基准
//1970~2099年为合法年份
//返回值:0,成功;其他:错误代码.
//月份数据表											 
u8 const table_week[12] = {0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表

//平年的月份日期表
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

/**
 * @brief 修改 RTC 时间
 * 		  RTC 时间转换时间戳
 * 
 * @param syear,smon,sday,hour,min,sec：年月日时分秒
 * @return u8 设置结果。0，成功；	1，失败。
 */
u8 RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
    u16 t;
    u32 seccount = 0;	// 1970 到现在的秒数 写入寄存器
    if(syear < 1970 || syear > 2099)
        return 1;	   

    for(t = 1970; t < syear; t++)	//把所有年份的秒钟相加
    {
        if(Is_Leap_Year(t))
            seccount += 31622400;//闰年的秒钟数
        else 
            seccount += 31536000;			  //平年的秒钟数
    }
    smon -= 1;
    for(t = 0; t < smon; t++)	   //把前面月份的秒钟数相加
    {
        seccount += (u32)mon_table[t]*86400;//月份秒钟数相加
        if(Is_Leap_Year(syear) && t == 1)
            seccount += 86400;//闰年2月份增加一天的秒钟数	   
    }

    seccount += (u32)(sday-1)*86400;//把前面日期的秒钟数相加 
    seccount += (u32)hour*3600;//小时秒钟数
    seccount += (u32)min*60;	 //分钟秒钟数
    seccount += sec;//最后的秒钟加上去
                                                        
    //设置时钟
    RCC->APB1ENR |= 1 << 28;//使能电源时钟
    RCC->APB1ENR |= 1 << 27;//使能备份时钟
    PWR->CR |= 1 << 8;    //取消备份区写保护, 关闭后备区域写保护

    //上面三步是必须的!
    RTC->CRL |= 1 << 4;   //允许配置 
    RTC->CNTL = seccount & 0xffff; // 低 2 byts
    RTC->CNTH = seccount >> 16; // 高 2 byte
    RTC->CRL &= ~(1<<4);//配置更新
    while(!(RTC->CRL&(1<<5)))
        ;//等待RTC寄存器操作完成 
    RTC_Get();//设置完之后更新一下数据 	

    LOG_I("rtc time update:%d-%d-% %d:%d:%d", syear, smon, sday, hour, min, sec);
    return 0;	    
}

/**
 * @brief 通过从 1970 开始的秒钟数, 获取 RTC 时间
 * 		  得到当前的时间，结果保存在 calendar 结构体里面
 * 		  (1) 获取时间, 时间戳转换为国际时间;
 * 
 * @return u8 返回值:0,成功;其他:错误代码.
 */
u8 RTC_Get(void)
{
    static u16 daycnt=0;
    u32 timecount=0; 
    u32 temp=0;
    u16 temp1=0;	  
    timecount = RTC->CNTH;//得到计数器中的值(秒钟数)
    timecount <<= 16;
    timecount += RTC->CNTL; // += ???	|= 

    temp = timecount / 86400;   //得到天数(秒钟数对应的)
    if(daycnt!=temp)//超过一天了
    {
        daycnt=temp;
        temp1=1970;	//从1970年开始
        while(temp>=365)
        {
            if(Is_Leap_Year(temp1))//是闰年
            {
                if(temp>=366)temp-=366;//闰年的秒钟数
                else break;  
            }
            else temp-=365;	  //平年 
            temp1++;  
        }   
        calendar.w_year=temp1;//得到年份
        temp1=0;
        while(temp>=28)//超过了一个月
        {
            if(Is_Leap_Year(calendar.w_year)&&temp1==1)//当年是不是闰年/2月份
            {
                if(temp>=29)temp-=29;//闰年的秒钟数
                else break; 
            }
            else 
            {
                if(temp>=mon_table[temp1])temp-=mon_table[temp1];//平年
                else break;
            }
            temp1++;  
        }
        calendar.w_month=temp1+1;	//得到月份
        calendar.w_date=temp+1;  	//得到日期 
    }

    temp = timecount % 86400;     		//得到秒钟数   	   
    calendar.hour = temp / 3600;     	//小时
    calendar.min = (temp % 3600) / 60; 	//分钟	
    calendar.sec = (temp % 3600) % 60; 	//秒钟
    calendar.week=RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);//获取星期   
    return 0;
}

//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//year,month,day：公历年月日 
//返回值：星期号																						 
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{
    u16 temp2;
    u8 yearH,yearL;
    
    yearH=year/100;	yearL=year%100; 
    // 如果为21世纪,年份数加100  
    if (yearH>19)yearL+=100;
    // 所过闰年数只算1900年之后的  
    temp2=yearL+yearL/4;
    temp2=temp2%7; 
    temp2=temp2+day+table_week[month-1];
    if (yearL%4==0&&month<3)temp2--;
    return(temp2%7);
}			  
















