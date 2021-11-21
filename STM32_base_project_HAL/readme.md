##### 1.后续考虑添加 service 层，实现底层和上层应用分层

##### 2.位带操作（bit band）
#define DEVICE_REG0 ((volatile unsigned long*)(0x40000000))
#define DEVICE_REG0_BIT0 ((volatile unsigned long*)(0x40000000))
#define DEVICE_REG0_BIT1 ((volatile unsigned long*)(0x40000000))

*DEVICE_REG0 = 0xAB;        //使用正常地址访问寄存器
*DEVICE_REG0 = *DEVICE_REG0  | 0x02;    //传统方法设置 bit1, 
*DEVICE_REG0 = 0x01;    //位带方法设置 bit1, 

为了简化位带操作, 也可以定义宏, 通过一个 (位带地址+位序号),将寄存器地址转换成别名地址，再建立一个把“位带地址”转换成指针类型变量的宏
#define BITBAND(addr, bitnum)   ((addr & 0xF0000000) + 0x20000000 + ((addr & 0xFFFFF) << 5) + (bitnum << 2))
//将地址转换为指针类型的变量
#define MEM_ADDR(addr) *((volatile unsigned long*)(addr))
MEM_ADDR(DEVICE_REG0) = 0xAB;
MEM_ADDR(DEVICE_REG0) = MEM_ADDR(DEVICE_REG0) | 0x2;    //传统做法
MEM_ADDR(BITBAND(DEVICE_REG0, 1)) = 0x1;        //使用位带别名地址
//使用位带操作时, 必须声明为 volatile 防止被编译器优化

##### 3.低功耗模式可以操作一下
(1)睡眠模式
(2)停止模式
(3)待机模式







