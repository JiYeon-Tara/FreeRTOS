/**
 * @file touch.c
 * @author your name (you@domain.com)
 * @brief touch panel bsp
 * @version 0.1
 * @date 2022-12-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdlib.h>
#include <math.h>

#include "touch.h" 
#include "spi2_soft.h"
#include "ILI93xx.h"
#include "delay.h"
#include "24cxx.h"
#include "ulog.h"


// ILI9341 使用电阻屏
// 使用软件 GPIO 模式 spi 方式


/*********************************************************************************
 * MACRO
 *********************************************************************************/
#define READ_TIMES 5 	//读取次数
#define LOST_VAL 1	  	//丢弃值

#define ERR_RANGE       50 //误差范围

//保存在EEPROM里面的地址区间基址,
//占用13个字节(RANGE:EEPROM_SAVE_TP_CAL_ADDR_BASE~EEPROM_SAVE_TP_CAL_ADDR_BASE+12)
#define EEPROM_SAVE_TP_CAL_ADDR_BASE 40

_m_tp_dev tp_dev = {
    TP_Init,
    TP_Scan,
    TP_Adjust,
    0,
    0, 
    0,
    0,
    0,
    0,	  	 		
    0,
    0,	  	 		
};

//默认为touchtype=0的数据. 默认使用电阻屏, ILI9341 也使用的电阻屏
//没有丰富的操作命令可以供操作, 只有简单的读取 X/Y 坐标命令
u8 CMD_RDX = 0XD0;
u8 CMD_RDY = 0X90;

//提示字符串
u8* const TP_REMIND_MSG_TBL="Please use the stylus click the cross on the screen.The cross will always move until the screen adjustment is completed.";


/**
 * @brief 从触摸屏IC读取adc值, 读取AD转换值
 * 
 * @param CMD 
 * @return u16 
 */
u16 TP_Read_AD(u8 CMD)
{
    SPI2_Soft_Read_AD(CMD);
}

/**
 * @brief 读取一个坐标值(x或者y)
 *        连续读取READ_TIMES次数据,对这些数据升序排列,
 *        然后去掉最低和最高LOST_VAL个数,取平均值 
 * 
 * @param xy 指令（CMD_RDX/CMD_RDY）
 * @return u16 读到的数据
 */
u16 TP_Read_XOY(u8 xy)
{
    u16 i, j;
    u16 buf[READ_TIMES];
    u16 sum=0;
    u16 temp;

    for(i = 0; i < READ_TIMES; i++)
        buf[i] = TP_Read_AD(xy);		 		    
    for(i = 0; i < READ_TIMES - 1; i++){ // 冒泡排序
        for(j = i + 1; j < READ_TIMES; j++){
            if(buf[i] > buf[j]){ //升序排列
                temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }
    sum = 0;
    for(i = LOST_VAL; i < READ_TIMES - LOST_VAL; i++)
        sum += buf[i];
    temp = sum / (READ_TIMES - 2 * LOST_VAL);

    return temp;   
}

/**
 * @brief 读取x,y坐标
 *        最小值不能少于100.
 * 
 * @param x 读取到的坐标值
 * @param y 
 * @return u8 0,失败;1,成功。
 */
u8 TP_Read_XY(u16 *x, u16 *y)
{
    u16 xtemp, ytemp;
    xtemp = TP_Read_XOY(CMD_RDX);
    ytemp = TP_Read_XOY(CMD_RDY);	  												   
    //if(xtemp<100||ytemp<100)
        // return 0;//读数失败
    *x = xtemp;
    *y = ytemp;

    return 1;//读数成功
}

/**
 * @brief 连续2次读取触摸屏IC,且这两次的偏差不能超过
 *        ERR_RANGE,满足条件,则认为读数正确,否则读数错误.
 *        该函数能大大提高准确度
 * 	  
 * @param x 读取到的坐标值
 * @param y 
 * @return * u8 0,失败;1,成功。
 */
u8 TP_Read_XY2(u16 *x,u16 *y)
{
    u16 x1,y1;
    u16 x2,y2;
    u8 flag;   

    flag = TP_Read_XY(&x1, &y1);   
    if(flag == 0)
        return (0);
    flag = TP_Read_XY(&x2, &y2);	   
    if(flag == 0)
        return(0);   
    if(((x2 <= x1 && x1 < x2 + ERR_RANGE) || (x1 <= x2 && x2 < x1 + ERR_RANGE))//前后两次采样在+-50内
    && ((y2 <= y1 && y1 < y2 + ERR_RANGE) || (y1 <= y2 && y2 < y1 + ERR_RANGE)))
    {
        *x = (x1 + x2) / 2;
        *y = (y1 + y2) / 2;
        return 1;
    }
    else 
        return 0;	  
}  

//////////////////////////////////////////////////////////////////////////////////		  
#if LCD_SCREEN_ENABLE == 1
//与LCD部分有关的函数  
//画一个触摸点
//用来校准用的
//x,y:坐标
//color:颜色
void TP_Drow_Touch_Point(u16 x, u16 y, u16 color)
{
    POINT_COLOR = color;
    LCD_DrawLine(x-12,y,x+13,y);//横线
    LCD_DrawLine(x,y-12,x,y+13);//竖线
    LCD_DrawPoint(x+1,y+1);
    LCD_DrawPoint(x-1,y+1);
    LCD_DrawPoint(x+1,y-1);
    LCD_DrawPoint(x-1,y-1);
    LCD_Draw_Circle(x,y,6);//画中心圈
}

//画一个大点(2*2的点), 占用 4 个像素点	   
//x,y:坐标
//color:颜色
void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
{
    POINT_COLOR=color;
    LCD_DrawPoint(x,y);//中心点 
    LCD_DrawPoint(x+1,y);
    LCD_DrawPoint(x,y+1);
    LCD_DrawPoint(x+1,y+1);	 	  	
}
#endif /* LCD_SCREEN_ENABLE */
//////////////////////////////////////////////////////////////////////////////////

//触摸按键扫描
//tp:0,屏幕坐标;1,物理坐标(校准等特殊场合用)
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
u8 TP_Scan(u8 tp)
{
    if (PEN == 0) { // 有按键按下
        if(tp) // 读取物理坐标
            TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]);
        else if(TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]))//读取屏幕坐标
        {
            tp_dev.x[0] = tp_dev.xfac * tp_dev.x[0] + tp_dev.xoff; //将结果转换为屏幕坐标
            tp_dev.y[0] = tp_dev.yfac * tp_dev.y[0] + tp_dev.yoff;  
        }

        if((tp_dev.sta & TP_PRES_DOWN) == 0)//之前没有被按下
        {		 
            tp_dev.sta = TP_PRES_DOWN | TP_CATH_PRES;//按键按下  
            tp_dev.x[4] = tp_dev.x[0];//记录第一次按下时的坐标
            tp_dev.y[4] = tp_dev.y[0];  	   			 
        }			   
    } else{
        if(tp_dev.sta & TP_PRES_DOWN)//之前是被按下的
        {
            tp_dev.sta &= ~(1 << 7); // clear, 标记按键松开	
        } else//之前就没有被按下
        {
            tp_dev.x[4] = 0;
            tp_dev.y[4] = 0;
            tp_dev.x[0] = 0xffff;
            tp_dev.y[0] = 0xffff;
        }	    
    }

    return tp_dev.sta & TP_PRES_DOWN;//返回当前的触屏状态
}

//
/**
 * @brief EEPROM 中保存校准参数	
 *        起始地址:40, 0x32
 * 
 * @return * void 
 */
void TP_Save_Adjdata(void)
{
    s32 temp;

    //保存校正结果!		   							  
    temp = tp_dev.xfac * 100000000;//保存x校正因素      
    AT24CXX_WriteLenByte(EEPROM_SAVE_TP_CAL_ADDR_BASE, temp, 4);   
    temp = tp_dev.yfac * 100000000;//保存y校正因素    
    AT24CXX_WriteLenByte(EEPROM_SAVE_TP_CAL_ADDR_BASE + 4, temp, 4);
    //保存x偏移量
    AT24CXX_WriteLenByte(EEPROM_SAVE_TP_CAL_ADDR_BASE + 8, tp_dev.xoff, 2);		    
    //保存y偏移量
    AT24CXX_WriteLenByte(EEPROM_SAVE_TP_CAL_ADDR_BASE + 10, tp_dev.yoff, 2);	
    //保存触屏类型
    AT24CXX_WriteOneByte(EEPROM_SAVE_TP_CAL_ADDR_BASE + 12, tp_dev.touchtype);	
    temp = 0X0A;//标记校准过了
    AT24CXX_WriteOneByte(EEPROM_SAVE_TP_CAL_ADDR_BASE + 13, temp); 
    LOG_I("save TP adjust parameter:%f %f %d %d 0x%02X", tp_dev.xfac, tp_dev.yfac, 
            tp_dev.xoff, tp_dev.yoff, tp_dev.touchtype);
    return;
}

//得到保存在EEPROM里面的校准值
//返回值：1，成功获取数据
//        0，获取失败，要重新校准
u8 TP_Get_Adjdata(void)
{
    s32 tempfac;
    tempfac=AT24CXX_ReadOneByte(EEPROM_SAVE_TP_CAL_ADDR_BASE+13);//读取标记字,看是否校准过！ 		 
    if(tempfac==0X0A)//触摸屏已经校准过了			   
    {
        tempfac=AT24CXX_ReadLenByte(EEPROM_SAVE_TP_CAL_ADDR_BASE,4);		   
        tp_dev.xfac=(float)tempfac/100000000;//得到x校准参数
        tempfac=AT24CXX_ReadLenByte(EEPROM_SAVE_TP_CAL_ADDR_BASE+4,4);			          
        tp_dev.yfac=(float)tempfac/100000000;//得到y校准参数
        //得到x偏移量
        tp_dev.xoff=AT24CXX_ReadLenByte(EEPROM_SAVE_TP_CAL_ADDR_BASE+8,2);			   	  
        //得到y偏移量
        tp_dev.yoff=AT24CXX_ReadLenByte(EEPROM_SAVE_TP_CAL_ADDR_BASE+10,2);				 	  
        tp_dev.touchtype=AT24CXX_ReadOneByte(EEPROM_SAVE_TP_CAL_ADDR_BASE+12);//读取触屏类型标记
        if(tp_dev.touchtype)//X,Y方向与屏幕相反
        {
            CMD_RDX=0X90;
            CMD_RDY=0XD0;	 
        } else //X,Y方向与屏幕相同
        {
            CMD_RDX=0XD0;
            CMD_RDY=0X90;	 
        }		 
        return 1;	 
    }
    LOG_I("tp get adjust parameter:%f %f %d %d 0x%02X", tp_dev.xfac, tp_dev.yfac, 
            tp_dev.xoff, tp_dev.yoff, tp_dev.touchtype);

    return 0;
}
                   
//提示校准结果(各个参数)
void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac)
{
    POINT_COLOR=RED;
    LCD_ShowString(40,160,lcddev.width,lcddev.height,16,"x1:");
    LCD_ShowString(40+80,160,lcddev.width,lcddev.height,16,"y1:");
    LCD_ShowString(40,180,lcddev.width,lcddev.height,16,"x2:");
    LCD_ShowString(40+80,180,lcddev.width,lcddev.height,16,"y2:");
    LCD_ShowString(40,200,lcddev.width,lcddev.height,16,"x3:");
    LCD_ShowString(40+80,200,lcddev.width,lcddev.height,16,"y3:");
    LCD_ShowString(40,220,lcddev.width,lcddev.height,16,"x4:");
    LCD_ShowString(40+80,220,lcddev.width,lcddev.height,16,"y4:");  
    LCD_ShowString(40,240,lcddev.width,lcddev.height,16,"fac is:");     
    LCD_ShowNum(40+24,160,x0,4,16);		//显示数值
    LCD_ShowNum(40+24+80,160,y0,4,16);	//显示数值
    LCD_ShowNum(40+24,180,x1,4,16);		//显示数值
    LCD_ShowNum(40+24+80,180,y1,4,16);	//显示数值
    LCD_ShowNum(40+24,200,x2,4,16);		//显示数值
    LCD_ShowNum(40+24+80,200,y2,4,16);	//显示数值
    LCD_ShowNum(40+24,220,x3,4,16);		//显示数值
    LCD_ShowNum(40+24+80,220,y3,4,16);	//显示数值
    LCD_ShowNum(40+56,240,fac,3,16); 	//显示数值,该数值必须在95~105范围之内.
}
         

/**
 * @brief 触摸屏校准代码
 *        得到四个校准参数 
 *        电阻屏需要校准, 电容屏不需要
 * 
 */
void TP_Adjust(void)
{
    u16 pos_temp[4][2];//坐标缓存值
    u8  cnt = 0;	
    u16 d1, d2;
    u32 tem1, tem2;
    double fac; 	
    u16 outtime = 0;

    LOG_I("TP adjust...\r\n");
    cnt = 0;
    POINT_COLOR = BLUE;
    BACK_COLOR = WHITE;
    LCD_Clear(WHITE);//清屏   
    POINT_COLOR = RED;//红色 
    LCD_Clear(WHITE);//清屏 	   
    POINT_COLOR = BLACK;
    LCD_ShowString(40, 40, 160, 100, 16, (u8*)TP_REMIND_MSG_TBL);//显示提示信息
    TP_Drow_Touch_Point(20, 20, RED);//画点1 
    tp_dev.sta = 0; //消除触发信号 
    tp_dev.xfac = 0; //xfac用来标记是否校准过,所以校准之前必须清掉!以免错误	 
    while(1)//如果连续10秒钟没有按下,则自动退出
    {
        tp_dev.scan(1);//扫描物理坐标
        if((tp_dev.sta & 0xc0) == TP_CATH_PRES)//按键按下了一次(此时按键松开了.)
        {
            outtime = 0;		
            tp_dev.sta &= ~(1 << 6);//标记按键已经被处理过了.
                                          
            pos_temp[cnt][0] = tp_dev.x[0];
            pos_temp[cnt][1] = tp_dev.y[0];
            cnt++;	  
            switch(cnt)
            {
                case 1:						 
                    TP_Drow_Touch_Point(20, 20, WHITE);				//清除点1 
                    TP_Drow_Touch_Point(lcddev.width - 20, 20, RED);	//画点2
                    break;
                case 2:
                    TP_Drow_Touch_Point(lcddev.width - 20, 20, WHITE);	//清除点2
                    TP_Drow_Touch_Point(20, lcddev.height - 20, RED);	//画点3
                    break;
                case 3:
                     TP_Drow_Touch_Point(20, lcddev.height-20, WHITE);			//清除点3
                     TP_Drow_Touch_Point(lcddev.width-20, lcddev.height - 20, RED);	//画点4
                    break;
                case 4:	 //全部四个点已经得到
                    //对边相等
                    tem1 = abs(pos_temp[0][0] - pos_temp[1][0]);//x1-x2
                    tem2 = abs(pos_temp[0][1] - pos_temp[1][1]);//y1-y2
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d1 = sqrt(tem1 + tem2);//得到1,2的距离
                    
                    tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
                    tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
                    tem1*=tem1;
                    tem2*=tem2;
                    d2=sqrt(tem1+tem2);//得到3,4的距离
                    fac=(float)d1/d2;
                    if(fac<0.95||fac>1.05||d1==0||d2==0)//不合格
                    {
                        cnt=0;
                         TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
                            TP_Drow_Touch_Point(20,20,RED);								//画点1
                         TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
                         continue;
                    }
                    tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
                    tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
                    tem1*=tem1;
                    tem2*=tem2;
                    d1=sqrt(tem1+tem2);//得到1,3的距离
                    
                    tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
                    tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
                    tem1*=tem1;
                    tem2*=tem2;
                    d2=sqrt(tem1+tem2);//得到2,4的距离
                    fac=(float)d1/d2;
                    if(fac<0.95||fac>1.05)//不合格
                    {
                        cnt=0;
                         TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
                            TP_Drow_Touch_Point(20,20,RED);								//画点1
                         TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
                        continue;
                    }//正确了
                                   
                    //对角线相等
                    tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
                    tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
                    tem1*=tem1;
                    tem2*=tem2;
                    d1=sqrt(tem1+tem2);//得到1,4的距离
    
                    tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
                    tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
                    tem1*=tem1;
                    tem2*=tem2;
                    d2=sqrt(tem1+tem2);//得到2,3的距离
                    fac=(float)d1/d2;
                    if(fac<0.95||fac>1.05)//不合格
                    {
                        cnt=0;
                         TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
                            TP_Drow_Touch_Point(20,20,RED);								//画点1
                         TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
                        continue;
                    }//正确了
                    //计算结果
                    tp_dev.xfac=(float)(lcddev.width-40)/(pos_temp[1][0]-pos_temp[0][0]);//得到xfac		 
                    tp_dev.xoff=(lcddev.width-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//得到xoff
                          
                    tp_dev.yfac=(float)(lcddev.height-40)/(pos_temp[2][1]-pos_temp[0][1]);//得到yfac
                    tp_dev.yoff=(lcddev.height-tp_dev.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//得到yoff  
                    if(abs(tp_dev.xfac)>2||abs(tp_dev.yfac)>2)//触屏和预设的相反了.
                    {
                        cnt=0;
                        TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
                        TP_Drow_Touch_Point(20,20,RED);								//画点1
                        LCD_ShowString(40,26,lcddev.width,lcddev.height,16,"TP Need readjust!");
                        tp_dev.touchtype=!tp_dev.touchtype;//修改触屏类型.
                        if(tp_dev.touchtype)//X,Y方向与屏幕相反
                        {
                            CMD_RDX=0X90;
                            CMD_RDY=0XD0;	 
                        }else				   //X,Y方向与屏幕相同
                        {
                            CMD_RDX=0XD0;
                            CMD_RDY=0X90;	 
                        }			    
                        continue;
                    }		
                    POINT_COLOR=BLUE;
                    LCD_Clear(WHITE);//清屏
                    LCD_ShowString(35,110,lcddev.width,lcddev.height,16,"Touch Screen Adjust OK!");//校正完成
                    delay_ms(1000);
                    TP_Save_Adjdata();  
                     LCD_Clear(WHITE);//清屏   
                    return;//校正完成				 
            }
        }
        delay_ms(10);
        outtime++;
        if(outtime > 1000)
        {
            TP_Get_Adjdata();
            break;
        } 
     }
}
    
/**
 * @brief 触摸屏初始化, 通过 LCD 型号判断 TP 型号
 *        需要在 LCD 后初始化
 * 
 * @return u8 0,没有进行校准; 1,进行过校准
 */
u8 TP_Init(void)
{
    LOG_I("TP Init\r\n");
    /* 电容屏不需要校准 */
    if (lcddev.id == 0X5510) { //4.3寸电容触摸屏
        if(GT9147_Init() == 0) //是GT9147
        { 
            tp_dev.scan = GT9147_Scan;	//扫描函数指向GT9147触摸屏扫描
        } else {
            OTT2001A_Init();
            tp_dev.scan = OTT2001A_Scan; //扫描函数指向OTT2001A触摸屏扫描
        }
        tp_dev.touchtype |= 0X80; // bit[7]:电容屏 
        tp_dev.touchtype |= lcddev.dir & 0X01;// bit[0]:横屏还是竖屏 
        return 0;
    } else if(lcddev.id==0X1963) { //7寸电容触摸屏
        FT5206_Init();
        tp_dev.scan=FT5206_Scan; //扫描函数指向GT9147触摸屏扫描		
        tp_dev.touchtype|=0X80; //电容屏 
        tp_dev.touchtype|=lcddev.dir&0X01;//横屏还是竖屏 
        return 0;
    } else { /* 电阻屏, 需要校准 */
        SPI2_Soft_Init(); // GPIO 软件模拟 SPI
        TP_Read_XY(&tp_dev.x[0],&tp_dev.y[0]);//第一次读取初始化
#if EEPROM_ENABLE
        AT24CXX_Init(); // 初始化 EEPROM 24CXX, EEPROM 中保存一些 TP 校准数据
#endif
        if(TP_Get_Adjdata()){ // TP 已经校准
            LOG_I("TP already adjusted.\r\n");
            return 0;
        } else { //TP 未校准
            LCD_Clear(WHITE);//清屏
            TP_Adjust();  	//屏幕校准 
            TP_Save_Adjdata();	 
        }
        TP_Get_Adjdata();	
    }

    LOG_I("TP init success touchtype:%02X\r\n", tp_dev.touchtype);
    return 1; 									 
}

