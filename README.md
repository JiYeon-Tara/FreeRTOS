##### STM32 学习——寄存器版本

TODO:

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



##### 4. stm32f103RCT6 这个板子的WK_UP按键好像有点问题，一直被触发，

answer:按键没有问题，PA0 不需要短接，和 DS18B20 不能共用。



##### 5. FreeeRTOS操作系统中各种线程同步的使用方法以及使用场景，参考当前项目

写到STM32_FreeRTOS分支



##### 6.STM32 的一键下载电路原理

供电，UART-USB 通信，下载电路三位一体。

一键下载电路的实现原理:



##### 7.flash 操作

flash page 划分：参考 STM32 数据手册， ARM 只是规定一个大致范围，真正的 page 分布，由芯片厂商自己去定义。



##### 8.FATFS 移植

http://elm-chan.org/fsw/ff/00index_e.html

U 盘上的文件系统需要格式化以后才可以在 STM32 上挂载成功???



##### 9.FreeRTOS 移植

https://www.freertos.org/zh-cn-cmn-s/



##### 10. UCOS 移植



##### 11. Nuttx 移植

https://nuttx.apache.org/



##### 12. OTA(串口 IAP)

boot.bin

app.bin

ota.bin:对 user_flash 进行更新