##### STM32 学习——寄存器版本

TODO:
 写 FATFS API 测试用例;
 service_fs_api 实现, 对 FATFS 等其他文件系统接口进行封装;
2. 实现 BMP, GIF, JPG 格式图片头信息打印函数;


书签:
1. ch29, 无线通信实验,使用 2.4GHz,但是没有使用蓝牙协议, 通信距离:100m;
2. OLED & LCD;
3. ch35~ch36. 字库(编解码)存储以及图片存储(编解码);
 ch41~ch43:UCOS FreeRTOS 学习联系完再看这部分;
4. ch38~ch39，学完"蓝牙， NFC52832 & esp32" 后再开始这部分 《USB》, 《CAN》;


5. MPU6050 姿态解算;
6. 定时器章节, 输入捕获/PWM输出功能, 红外遥控器章节;



从寄存器版本开始学习，后面库函数版本以及HAL库版本带过即可；

项目开发以及后续自己开发做东西肯定是要直接用 HAL 库的，但是学习还是要以原理为主，学的透彻、通透一点。加油~

先使用 Keil 编译版本 -> GCC 编译版本


Questions:
- 稚晖君 holocubic  + 华为太空人表盘；
- 后续可以添加蓝牙 OTA 以及使用手机更换图片的功能；



- **TODO:明日学习计划:ch35, ch36, ch15, ch16， TP——OLED 和 LCD 显示的详细学习，可以放晚点，开始学习其他内容，本周六、周天花两天时间搞定, LVGL:**
- 学有余力的情况下可以学习 freeRTOS, nuttx 以及 repo;
- LVGL 学到了各种 widgets 章节，还停留在 PC 仿真器阶段；
- DHT11 温湿度传感器——单总线通讯方式，使用自己定的简单通信协议；




##### 1. CM3 的中断管理

```C
unsigned int a;	//unsigned int 类型加到最大值 INT_MAX 之后, 会从0重新开始, int 就没有这种特性
uint8_t b;	// 0-255, int8_t -128 - 127
```



##### 2.移位操作究竟是什么原理

```C
#include <stdint.h>

int a = 1;
uint8 _t x = 1;
int8_t y = 1;

x << 1;
y << 30
```



##### 3.进制转换

itoa函数的原型为char* itoa(int value, char * string, int radix); int value 被转换的整数，char *string 转换后储存的字符数组int radix 转换进制数，如2,8,10,16 进制等，

```C
#include <stdlib.h>
void fun()
{
  	a = 12;
	char s[10];
  	itoa(a, s, 2);	//转换为二进制
  	printf("%s\n", s);
  	itoa(a, s, 3);
  	printf("%s\n", s);	//三进制
}
```



##### 4. WK_UP 按键问题
Q:stm32f103RCT6 这个板子的WK_UP按键好像有点问题，一直被触发，
A:按键没有问题，PA0 不需要短接，和 DS18B20 不能共用。



##### 5. FreeeRTOS操作系统中各种线程同步的使用方法以及使用场景，参考当前项目

写到STM32_FreeRTOS分支
TODO:
SVC 与 pendSV


##### 6.STM32 的一键下载电路原理

供电，UART-USB 通信，下载电路三位一体。

一键下载电路的实现原理:



##### 7.flash 操作

flash page 划分：
参考 STM32 数据手册， ARM 只是规定一个大致范围，真正的 page 分布，由芯片厂商自己去定义。
ARM-CortexM3 规定的 flash 0.5GB, 范围:0x00000000~0x1FFFFFFF
STM32F103RCT6:
片上 flash:
256KB
片外扩展 flash W25Q64:
8MByte

SRAM ARM 规定范围:0x20000000~0x3FFFFFFF:
48KB(0xC000)
0x20000000(0x20000000~0x2000BFFF)

##### 8.FATFS 移植

http://elm-chan.org/fsw/ff/00index_e.html

Q:U 盘上的文件系统需要格式化以后才可以在 STM32 上挂载成功???
A:FATFS 文件系统系统内部需要对磁盘进行分区, 分为系统区,用户区等, 一块空白的磁盘是无法直接挂载文件系统的, 需要先格式化分区;


##### 9.FreeRTOS 移植

https://www.freertos.org/zh-cn-cmn-s/



##### 10. UCOS 移植



##### 11. Nuttx(Vela) 移植

https://nuttx.apache.org/

##### 11. Linux 移植


##### 12. OTA(串口 IAP)

TODO:
1. 
boot.bin

app.bin

ota.bin:对 user_flash 进行更新

2.XIP 概念以及实现




;function jump
compare_fun
	MOV R1,#0x0004
	CMP R1,#0x00		;compare MOVGT, MOVLT, MOVEQ
	
	MOVEQ PC,LR ; if R1==0x00, return
	;MOV PC,LR ; return

;function loop
loop	
	MOV R1,#0x0004
	CMP R1,#0x00
	B loop
	
;save program state register
save_cpsr
	B save cpsr
	
subroutine_1
	;PUSH {R0-R7,R12,LR}		;save register
								;other operation
	;POP {R0-R7,R12,LR}			;restore register
	;BX R14						;return
	
special_fun_register_test1
	MRS	R0,CPSR				;save CPSR to R0
	MSR CPSR,R0
	
;label must define at line head
NVIC_IRQ_SETEN0		EQU		0xE000E100	;define constant
NVIC_IRQ0_ENABLE	EQU		0x01

constant_test1
	MOV R0,=NVIC_IRQ_SETN0
	MOV R1,#NVIC_IRQ0_ENABLE
	STR R1,[R0]					;*R0=R1,enable IRQ0 interrupt
	

number_test
	LDR R3,=TEST_NUMBER1			;R3=TEST_NUMBER1
	;LDR R4
	
	
TEST_NUMBER1
	DCD 0x12345678				;assemble psedo command
HELLO_TEXT
	DCB "HELLO\n"0
	
	END
