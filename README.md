##### STM32 ѧϰ�����Ĵ����汾

TODO:

�ӼĴ����汾��ʼѧϰ������⺯���汾�Լ�HAL��汾�������ɣ�

��Ŀ�����Լ������Լ������������϶���Ҫֱ���� HAL ��ģ�����ѧϰ����Ҫ��ԭ��Ϊ����ѧ��͸����ͨ͸һ�㡣����~

��ʹ�� Keil ����汾 -> GCC ����汾

Questions:

- ���;� holocubic  + ��Ϊ̫���˱��̣�
- ��������������� OTA �Լ�ʹ���ֻ�����ͼƬ�Ĺ��ܣ�



- **TODO:����ѧϰ�ƻ�:ch35, ch36, ch15, ch16�� TP����OLED �� LCD ��ʾ����ϸѧϰ�����Է���㣬��ʼѧϰ�������ݣ������������컨����ʱ��㶨, LVGL:**
- ѧ������������¿���ѧϰ freeRTOS, nuttx �Լ� repo;
- LVGL ѧ���˸��� widgets �½ڣ���ͣ���� PC �������׶Σ�
- DHT11 ��ʪ�ȴ���������������ͨѶ��ʽ��ʹ���Լ����ļ�ͨ��Э�飻




##### 1. CM3 ���жϹ���

```C
unsigned int a;	//unsigned int ���ͼӵ����ֵ INT_MAX ֮��, ���0���¿�ʼ, int ��û����������
uint8_t b;	// 0-255, int8_t -128 - 127
```



##### 2.��λ����������ʲôԭ��

```C
#include <stdint.h>

int a = 1;
uint8 _t x = 1;
int8_t y = 1;

x << 1;
y << 30
```



##### 3.����ת��

itoa������ԭ��Ϊchar* itoa(int value, char * string, int radix); int value ��ת����������char *string ת���󴢴���ַ�����int radix ת������������2,8,10,16 ���Ƶȣ�

```C
#include <stdlib.h>
void fun()
{
  	a = 12;
	char s[10];
  	itoa(a, s, 2);	//ת��Ϊ������
  	printf("%s\n", s);
  	itoa(a, s, 3);
  	printf("%s\n", s);	//������
}
```



##### 4. stm32f103RCT6 ������ӵ�WK_UP���������е����⣬һֱ��������

answer:����û�����⣬PA0 ����Ҫ�̽ӣ��� DS18B20 ���ܹ��á�



##### 5. FreeeRTOS����ϵͳ�и����߳�ͬ����ʹ�÷����Լ�ʹ�ó������ο���ǰ��Ŀ

д��STM32_FreeRTOS��֧



##### 6.STM32 ��һ�����ص�·ԭ��

���磬UART-USB ͨ�ţ����ص�·��λһ�塣

һ�����ص�·��ʵ��ԭ��:



##### 7.flash ����

flash page ���֣��ο� STM32 �����ֲᣬ ARM ֻ�ǹ涨һ�����·�Χ�������� page �ֲ�����оƬ�����Լ�ȥ���塣



##### 8.FATFS ��ֲ

http://elm-chan.org/fsw/ff/00index_e.html

U ���ϵ��ļ�ϵͳ��Ҫ��ʽ���Ժ�ſ����� STM32 �Ϲ��سɹ�???



##### 9.FreeRTOS ��ֲ

https://www.freertos.org/zh-cn-cmn-s/



##### 10. UCOS ��ֲ



##### 11. Nuttx ��ֲ

https://nuttx.apache.org/



##### 12. OTA(���� IAP)

boot.bin

app.bin

ota.bin:�� user_flash ���и���