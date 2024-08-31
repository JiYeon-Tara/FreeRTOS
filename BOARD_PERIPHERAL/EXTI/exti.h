#ifndef __EXTI_H__
#define __EXTI_H__

#include "util.h"
#include "board_config.h"

void EXTI_Init(void);

int stm_key_pressed(void);
void sm_key_reset(void);


#endif

