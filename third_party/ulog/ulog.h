#ifndef __ULOG_H__
#define __ULOG_H__

#include "usart.h"

#define DEBUG          1

#ifdef DEBUG
//TODO:
// add tag
// #define LOG_D(tag)

#define LOG_D(fmt, ...)      printf("[D] [%s:%d] "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)
#define LOG_I(fmt, ...)      printf("[I] [%s:%d] "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)
#define LOG_W(fmt, ...)      printf("[W] [%s:%d] "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)
#define LOG_E(fmt, ...)      printf("[E] [%s:%d] "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)
#define LOG_HEX(tag, p_data, len)               \
    do {                                        \
        int i;                                  \
        int already_print = 0;                  \
        printf("%s(%d):\n", tag, len);          \
        while (already_print < len) {           \
            int row_char_num;                   \
            if ((len - already_print) > 20) {   \
                row_char_num = 20;              \
            } else {                            \
                row_char_num = len - already_print; \
            }                                   \
            for (i = 0; i < row_char_num ; ++i) { \
                printf("%02X", p_data[already_print + i]); \
            }                                   \
            printf(" | ");                   \
            for (i = 0; i < row_char_num; ++i) { \
                printf("%c", p_data[already_print + i]); \
            }                                   \
            printf("\n");                     \
            already_print += row_char_num;      \
        }                                       \
    } while (0)
#else
#define LOG_D(...)
#define LOG_I(...)      printf(##__VA_ARGS__)
#define LOG_W(...)      printf(##__VA_ARGS__)
#define LOG_E(...)      printf(##__VA_ARGS__)
#define LOG_HEX(tag, p_data, len)           \
    do {                                    \
        int i;                              \
        printf("%s", tag);                 \
        for (i = 0; i < len; ++i) {         \
            printf("%02X ", p_data[i]);     \
        }                                   \
        printf("    | ");                       \
        for (i = 0; i < len; ++i) {         \
            printf("%c ", p_data[i]);       \
        }                                   \
        printf("\n");                       \
    } while (0)
#endif

//TODO:
// ulog module port

#endif

