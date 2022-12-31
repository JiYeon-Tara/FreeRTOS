#include "ps2.h" 
#include "usart.h"


//PS2产生的时钟频率在10~20Khz(最大33K)
//高/低电平的持续时间为25~50us之间. 

// PS2_Status当前状态标志位
// bit[7]:接收到一次数据;
// bit[6]:校验错误;
// bit[5:4]:当前工作的模式;
// bit[3:0]:收到的数据长度, 0 - 16;		   
u8 PS2_Status = CMDMODE; //默认为命令模式 
u8 PS2_DATA_BUF[16];   //ps2数据缓存区

//位计数器
u8 BIT_Count = 0;

//中断 15~10 处理函数
//每 11 个bit,为接收 1 个字节
//每接收完一个包(11位)后,设备至少会等待 50ms 再发送下一个包
//只做了鼠标部分,键盘部分暂时未加入
// void EXTI15_10_IRQHandler(void)
// {
//     static u8 tempdata = 0;
//     static u8 parity = 0; // 用于奇偶校验

//     if(EXTI->PR & (1 << 15)){ //中断15产生了相应的中断
//         EXTI->PR = 1 << 15;  //清除LINE15上的中断标志位
//         if(BIT_Count == 0){
//             parity = 0;
//             tempdata = 0;
//         }

//         BIT_Count++;    
//         if(BIT_Count > 1 && BIT_Count < 10){ //这里获得数据  
//             tempdata >>= 1;
//             if(PS2_SDA){
//                 tempdata |= 0x80;
//                 parity++;//记录1的个数
//             }
//         }
//         else if(BIT_Count == 10){ //得到校验位
//             if(PS2_SDA)
//                 parity |= 0x80;//校验位为1
//         }	

//         if(BIT_Count == 11) {//接收到1个字节的数据了
//              BIT_Count = parity & 0x7f;//取得1的个数	  
//             if(((BIT_Count % 2 == 0) && (parity & 0x80)) || ((BIT_Count % 2 == 1) && (parity & 0x80) == 0))//奇偶校验OK
//             {					    
//                 //PS2_Status|=1<<7;//标记得到数据	   
//                 BIT_Count = PS2_Status & 0x0f;		
//                 PS2_DATA_BUF[BIT_Count] = tempdata;//保存数据
//                 if(BIT_Count < 15)
//                     PS2_Status++;    //数据长度加1
//                 BIT_Count = PS2_Status & 0x30;	     //得到模式	

//                 switch(BIT_Count){
//                 case CMDMODE://命令模式下,每收到一个字节都会产生接收完成
//                     PS2_Dis_Data_Report();//禁止数据传输
//                     PS2_Status |= 1 << 7; //标记得到数据
//                 break;
//                 case KEYBOARD:
//                 break;
//                 case MOUSE:
//                     if(MOUSE_ID == 0){ //标准鼠标,3个字节
//                         if((PS2_Status & 0x0f) == 3){
//                             PS2_Status |= 1 << 7;//标记得到数据
//                             PS2_Dis_Data_Report();//禁止数据传输
//                         }
//                     }
//                     else if(MOUSE_ID == 3){ //扩展鼠标,4个字节
//                         if((PS2_Status & 0x0f) == 4){
//                             PS2_Status |= 1 << 7;//标记得到数据
//                             PS2_Dis_Data_Report();//禁止数据传输
//                         }
//                     }	 
//                 break;
//                 }		   		 
//             }
//             else{ // 校验失败
//                 PS2_Status |= 1 << 6;//标记校验错误
//                 PS2_Status &= 0xf0;//清除接收数据计数器
//             }
//             BIT_Count = 0;
//         } 	 	  
//     }			  
// }

/**
 * @brief 禁止数据传输
 *        把时钟线拉低,禁止数据传输	   
 * 
 */
void PS2_Dis_Data_Report(void)
{
    PS2_Set_Int(0); //关闭中断
    PS2_SET_SCL_OUT();//设置SCL为输出
    PS2_SCL_OUT = 0; //抑制传输
}

/**
 * @brief 使能数据传输
 *        释放时钟线	
 * 
 * @return * void 
 */
void PS2_En_Data_Report(void)
{
    PS2_SET_SCL_IN(); //设置SCL为输入
    PS2_SET_SDA_IN(); //SDA IN
    PS2_SCL_OUT = 1; //上拉   
    PS2_SDA_OUT = 1; 
    PS2_Set_Int(1); //开启中断
}

/**
 * @brief PS2中断屏蔽设置
 * 
 * @param en 1，开启;0，关闭;
 */
void PS2_Set_Int(u8 en)
{
    EXTI->PR = 1 << 15;  //清除LINE15上的中断标志位
    if(en)
        EXTI->IMR |= 1 << 15;//不屏蔽line15上的中断
    else 
        EXTI->IMR &= ~(1 << 15);//屏蔽line15上的中断   
}


/**
 * @brief 等待PS2时钟线sta状态改变
 *        PS2 时钟是由鼠标/键盘产生
 * 
 * @param sta 1，等待变为1;0，等待变为0;
 * @return * u8 0，时钟线变成了sta; 1，超时溢出;
 */
u8 Wait_PS2_Scl(u8 sta)
{
    u16 t = 0;
    sta = !sta;
    while(PS2_SCL == sta){
        delay_us(1);
        t++;
        if(t > 16000) //时间溢出 (设备会在10ms内检测这个状态)
            return 1;
    }
    return 0; //等到了想要的电平
}

	 
/**
 * @brief 在发送命令/数据之后,等待设备应答,该函数用来获取应答
 *        返回得到的值 
 * 
 * @return * u8 返回0，且PS2_Status.6=1，则产生了错误
 */
u8 PS2_Get_Byte(void)
{
    u16 t = 0;
    u8 temp = 0;
    while(1){ //最大等待55ms
        t++;
        delay_us(10);
        if(PS2_Status & 0x80) // (PS2_Status & (1 << 7)), 得到了一次数据
        {
            temp = PS2_DATA_BUF[PS2_Status & 0x0f - 1]; // (PS2_Status & 0x0f) - 1
            PS2_Status &= 0x70;//清除计数器，接收到数据标记, 取一个字节然后把缓冲区的数据全部清掉?????????????????
            break;	
        }
        else if(t > 5500 || PS2_Status & 0x40) //超时溢出/接收错误
            break;
    }
    PS2_En_Data_Report();//使能数据传输
    return temp;    
}	

/**
 * @brief 发送一个命令到PS2.
 * 
 * @param cmd 
 * @return u8 0，无错误,其他,错误代码
 */
u8 PS2_Send_Cmd(u8 cmd)
{
    u8 i;
    u8 high=0;//记录1的个数		 
    PS2_Set_Int(0);   //屏蔽中断
    PS2_SET_SCL_OUT();//设置SCL为输出
    PS2_SET_SDA_OUT();//SDA OUT
    PS2_SCL_OUT = 0;//拉低时钟线
    delay_us(120);//保持至少100us
    // 起始位
    PS2_SDA_OUT = 0;// 拉低数据线, 作为起始位 START
    delay_us(10);
    PS2_SET_SCL_IN();//释放时钟线,这里PS2设备得到第一个位,开始位
    PS2_SCL_OUT = 1;
    if(Wait_PS2_Scl(0) == 0){ //等待时钟拉低, 数据在低电平的时候切换, 在上升沿的时候锁存, 被设备读取
        // 数据
        for(i = 0; i < 8; i++){
            if(cmd & 0x01){
                PS2_SDA_OUT = 1;
                high++;
            }
            else 
                PS2_SDA_OUT = 0;   
            cmd >>= 1;
            //这些地方没有检测错误,因为这些地方不会产生死循环
            Wait_PS2_Scl(1);//等待时钟拉高 发送8个位
            Wait_PS2_Scl(0);//等待时钟拉低
        }

        // 校验位
        if((high % 2) == 0) //发送校验位 10
            PS2_SDA_OUT = 1;
        else 
            PS2_SDA_OUT = 0;

        // 停止位
        Wait_PS2_Scl(1); //等待时钟拉高 10位
        Wait_PS2_Scl(0); //等待时钟拉低
        PS2_SDA_OUT = 1; //发送停止位  11	  

        // 等待设备回复 ACK
        Wait_PS2_Scl(1);//等待时钟拉高 11位
        PS2_SET_SDA_IN();//SDA in
        Wait_PS2_Scl(0);//等待时钟拉低
        if(PS2_SDA == 0)
            Wait_PS2_Scl(1);//等待时钟拉高 12位 
        else {
            PS2_En_Data_Report();
            return 1;//发送失败
        }		
    }
    else{
        PS2_En_Data_Report();
        return 2;//发送失败
    }
    PS2_En_Data_Report();
    return 0;    //发送成功
}

/**
 * @brief PS2初始化
 *        PS2_CLK - PA15; PS2_DATA - PC5
 * 
 * @return * void 
 */
void PS2_Init(void)
{
    RCC->APB2ENR |= 1 << 2;    //使能PORTA时钟
    RCC->APB2ENR |= 1 << 4;    //使能PORTC时钟
     GPIOA->CRH &= 0X0FFFFFFF;	
    GPIOA->CRH |= 0X80000000;//PA15设置成输入	
    GPIOA->ODR |= 1 << 15;	 
     GPIOC->CRL &= 0XFF0FFFFF;	
    GPIOC->CRL |= 0X00800000;//PC5设置成输入	
    GPIOC->ODR |= 1 << 5;	  
    GPIO_NVIC_Config(GPIO_A, 15, FTIR); //将line15映射到PA.15，下降沿触发.
    MY_NVIC_Init(1, 2, EXTI15_10_IRQn, 2);	//分配到第二组,抢占2,响应3		 
}

















