/**
 * @file test.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __TEST_H__
#define __TEST_H__
#include <stdio.h>
#include <stdlib.h>

#include "board_config.h"
#include "bsp_config.h"
// #include "service_fs_api.h"


void board_peripheral_init(void);
void bsp_init(void);
void service_init(void);

void board_peripheral_test_loop(void);
void bsp_test_loop(void);
void component_test_loop(void);


void eeprom_test(void);
void external_flash_test(uint8_t dir);
void inner_flash_test(uint8_t read_write_flag);
void memmang_test(uint8_t flag);
void sdcard_read_write_sectorx_test(void);
void dma_test(void);

#endif

