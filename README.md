##### STM32 学习——寄存器版本

从寄存器版本开始学习，后面库函数版本以及HAL库版本带过即可；

项目开发以及后续自己开发做东西肯定是要直接用 HAL 库的，但是学习还是要以原理为主，学的透彻、通透一点。加油~

先使用 Keil 编译版本 -> GCC 编译版本

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



##### 5. FreeeRTOS操作系统中各种线程同步的使用方法以及使用场景，参考当前项目

写到STM32_FreeRTOS分支