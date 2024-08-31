#include "util.h"
#include "test.h"
#include "ulog.h"

/*********************************************************************************
 * MACRO
 *********************************************************************************/

/*********************************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************************/

/*********************************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************************/

/*********************************************************************************
 * PROTOTYPE
 *********************************************************************************/


/*********************************************************************************
 * PUBLIC VARIABLES
 *********************************************************************************/

/*********************************************************************************
 * PRIVATE VARIABLES
 *********************************************************************************/
/**
 * @brief bit operation test, bit index start from 0
 * 
 */
void bit_oper_test(void)
{
    uint8_t a = SET_BIT_M(1); // bit[1]
    uint8_t b = SET_BIT_M(1) | SET_BIT_M(3); // bit[1]bit[3]
    uint8_t c = 0x0F, d = 0x0F;
    uint8_t e = 0, f = 0;


    c &= CLEAR_BIT_M(1);
    d &= CLEAR_BIT_M(1) & CLEAR_BIT_M(3);
    LOG_D("a = %#X", a);
    LOG_D("b = %#X", b);
    LOG_D("c = %#X", c);
    LOG_D("d = %#X", d);

    e &= CLEAR_BIT_M(1);
    f |= SET_BIT_M(1);
    LOG_D("e = %#X", e);
    LOG_D("f = %#X", f);

}

//
//x1,x2
//返回值：
/**
 * @brief 两个数之差的绝对值 
 * 
 * @param x1 ：需取差值的两个数
 * @param x2 
 * @return u16 |x1-x2|
 */
u16 my_abs(u16 x1, u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
} 

/**
 * @brief MSB First Change to LSB first
 *        字节中 bit 的顺序并没有变化, 由于有些数据是跨字节的, 所以不能使用该方法
 * 
 * @param arr 
 * @param len 
 * @return uint8_t 
 */
int reverse_byte(uint8_t *arr, uint32_t len)
{
    uint8_t *temp;
    uint32_t ix;

    if (!arr || !len)
        return -1;

    temp = (uint8_t*)mymalloc(len);
    if(!temp){
        LOG_I("malloc error\r\n");
        return -1;
    }

    mymemcpy(temp, (uint8_t*)arr, len);

    for(ix = 0; ix < len; ++ix){
        arr[ix] = temp[len - 1 - ix];
    }

    myfree(temp);

    return 0;
}

/**
 * @brief bit[127:0] -> bit[0:127], bit 顺序完全翻转
 * 
 * @param arr 
 * @param len 
 * @return uint8_t 
 */
int reverse_bit(uint8_t* arr, uint32_t len)
{
    uint8_t *temp;
    uint32_t ix;
    u8 elem;
    u8 jx;

    if (!arr || !len)
        return -1;

    temp = (uint8_t*)mymalloc(len);
    if(!temp){
        LOG_I("malloc error\r\n");
        return -1;
    }
    mymemcpy(temp, (uint8_t*)arr, len);

    for (ix = 0; ix < len; ++ix) {
        arr[ix] = ~temp[len - ix - 1];
    }

    return 0;
}
