#ifndef __SERVICE_DUAL_COMM_COMMON_H__
#define __SERVICE_DUAL_COMM_COMMON_H__

#include <stdbool.h> // std lib
#include <stdint.h>
#include <string.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_timer.h"

#include "boards.h"


//
// function declaration
//
void dual_comm_init(void);

#endif //end __SERVICE_DUAL_COMM_COMMON_H__


