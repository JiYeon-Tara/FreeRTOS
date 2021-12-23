/**
 * @file str_operation.c
 * @author your name (you@domain.com)
 * @brief 字符串操作
 * @version 0.1
 * @date 2021-12-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "str_operation.h"

/**
 * @brief 将字符串中的小写字母转换为大写字母
 * 
 * @param str 
 * @param len 
 */
void LowerToCapital(uint8_t *str, uint8_t len)
{
    uint8_t ix;
    for(ix = 0; ix < len; ++ix)
    {
        if(str[ix]>96 && str[ix]<123)
        {
            str[ix] = str[ix] - 32;
        }
    }

    return;
}

