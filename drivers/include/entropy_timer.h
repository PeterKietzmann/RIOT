/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup   entropy_timer Entopry from timer jitter
 * @ingroup    drivers_periph
 * @brief      Entropy driver from timer jitter
 * @{
 *
 * @warning    This API reqires blabla
 *
 * # Aout
 *
 *
 *
 * @file
 *
 * @author     Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */

#ifndef ENTROPY_TIMER_H
#define ENTROPY_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "periph_conf.h"
#include "periph/rtt.h"
#include "periph/timer.h"

#include "mutex.h"


#define RTT_TIMER_TICKS_WAIT (10)//(RTT_FREQUENCY)

#define PERIPH_TIMER_FREQ    (20000000U)
#define PERIPH_TIMER_DEV     (0)

/**
 * @brief   Context for entropy timer module
 */
typedef struct {
    uint8_t *pool;
    uint8_t count_meas;
    uint8_t count_bytes;
} entropy_timer_t;

/**
 * @brief   BLABLA entropy module
 *
 * @param[in] buf   abcd
 * @param[in] buf   efgh
 *
 * @return          X on ijhklmop
 * @return          Y on qrstuvwx
 */
int entropy_timer_get(uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* ENTROPY_TIMER_H */
