#include "service_asm_test.h"



/**
 * @brief LDR 可以直接访问的区域包括片上 flash 区域, 比如下面的加载全局变量
 * 代码区域:0x00000000 开始
 * RO-DATA & Code 是保存在 flash 中的;
 */
void service_asm_test_load_global_val(void)
{
    // PUSH {R4,LR}
    static int a = 0x5A;
    static int b = 2;
    int c = 0; // MOVS R4,0x00

    // 汇编
    // LDR R3,[0x00000000,offset]
    // LDR R0,[PC,#16] ;0x080118C0
    // LDR R4,[R0,0x00]
    c = a; 

    LOG_I("c = %d", c);
    return;
}

int nested_fun_call_fun1(int param1, int param2, int param3)
{
    int a = 1;
    int b = 2;
    int c = 3;
    int ret = -1;
    
    a = param1;
    b = param2;
    c = param3;
    if (a > 0)
        ret = nested_fun_call_fun2(a, b, c); // 参数通过 R0,R1,R2 传递
    else
        ret = nested_fun_call_fun3(a, b, c);
    
    ret = nested_fun_call_fun4(a, b, c, 1, 2, 3);

    return ret; // 返回值通过 R0 传递
}

int nested_fun_call_fun2(int param1, int param2, int param3)
{
    int ret = 2;
    LOG_I("param1:%d param2:%d param3:%d", param1, param2, param3);
    return ret;
}

int nested_fun_call_fun3(int param1, int param2, int param3)
{
    int ret;

    ret = param1 + param2 + param3;
    LOG_I("param1:%d param2:%d param3:%d", param1, param2, param3);
    return ret;
}

// 参数大于 4 个时需要入栈
int nested_fun_call_fun4(int param1, int param2, int param3, int param4, int param5, int param6)
{
    int ret;

    ret = param1 + param2 + param3 - param4 - param5 - param6;
    LOG_I("param1:%d param2:%d param3:%d param4:%d param5:%d param6:%d", 
        param1, param2, param3, param4, param5, param6);
    return ret;
}

int null_ptr_test(void *p, int len)
{
    int ret = -1;
    *(int*)p = len; // STR R1,[R2,#0x00], R2为 0x00 -> HardFaultHandler

    return ret;
}

// TODO: 
//为什么除 0 不会死机???
int div_0_test(void)
{
    int ret = -1;
    int a = 0;
    int b = 16;

    b = b / a;

    return ret;

}


