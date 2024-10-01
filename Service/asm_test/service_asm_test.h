#ifndef __SERVICE_ASM_TEST_H__
#define __SERVICE_ASM_TEST_H__

#include "util.h"

void service_asm_test_load_global_val(void);
int nested_fun_call_fun1(int param1, int param2, int param3);
int null_ptr_test(void *p, int len);
int div_0_test(void);


#endif

