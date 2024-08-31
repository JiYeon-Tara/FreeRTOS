/**
 * @file main.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// 注意:
// (1)如果 falsh 空间足够, 是可以存放很多不同的 app.bin 的;
// (2) STM32 的 APP 程序可以放到 FLASH 里面运行, 也可以放到 SRAM 里面运行;



// bootloader 代码需要满足两个条件
// 1） 新程序必须在 IAP 程序之后的某个偏移量为 x 的地址开始;
// 2） 必须将新程序的中断向量表相应的移动，移动的偏移量为 x;

// a. flash APP
// 片上 flash - 存放代码区
// address:0x08000000 - 0x08040000
// size:0x40000(256K)

// 增加 bootloader 以后, 这里 bootloader 的大小不是固定的, 可以根据 Bootloader 大小进行不同设置;
// address:0x08008000 - 0x08040000, 偏移了 0x8000(32K)
// size:256K - 32K = 224K


// b. SRAM app
// address:0x20000000 - 0x2000C000
// size:0xC000(48K)

// 增加 bootloader
// bootloader:0x20000000 - 0x20000FFF, 最开始的 4K bootloader;
// APP:0x20001000 - 0x2000AFFF, 中间的 40K 存放 APP 程序;
// 0x2000B000 - 0x2000BFFF, 最后的 4K 用做 APP 程序的内存

// address:0x20001000 - 0x2000B000
// size:44K


// bootloader 空间设置规则:
// 1， 保证偏移量为 0X200 的倍数（我们这里为 0X1000）。
// 2， IROM1 的容量最大为 41KB（因为 IAP 代码里面接收数组最大是 41K 字节）。
// 3， IROM1 的地址区域和 IRAM1 的地址区域不能重叠。
// 4， IROM1 大小+IRAM1 大小，不要超过 44KB（48K-4K）。
// 2.中断向量表的偏移量设置方法