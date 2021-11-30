/**
 * @file include_head.h
 * @author your name (you@domain.com)
 * @brief include file
 * @version 0.1
 * @date 2021-11-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __INCLUDE_HEAD_H__
#define __INCLUDE_HEAD_H__

//
// std lib
//
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//
// Nordic SDK
//


//
// log print
//
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

//
//service
//
#include "service_bsp.h"
#include "service_ble.h"
#include "service_bt.h"
#include "service_algo.h"
#include "service_dual_comm_common.h"

#endif  // end __INCLUDE_HEAD_H__

