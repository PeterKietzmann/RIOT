/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     entropy_timer
 *
 * @{
 * @file
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include "entropy_timer.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"

/**
 * @brief   Allocate memory for entropy timer
 */
static entropy_timer_t e_tim;

/**
 * @brief   Allocate mutex
 */
static mutex_t lock;

void _entropy_timer_init(void)
{
    e_tim.count_meas = 0;
    e_tim.count_bytes = 0;

    mutex_init(&lock);
}

void _entropy_timer_rtt_cb(void *arg)
{
    entropy_timer_t *e_tim_arg = (entropy_timer_t *)arg;

    if (e_tim_arg->count_meas > 0) {
        uint32_t rtt_now, rtt_wakeup;
        uint8_t time, shift;

        rtt_now = rtt_get_counter();
        rtt_wakeup = (rtt_now + RTT_TIMER_TICKS_WAIT) & RTT_MAX_VALUE;
        rtt_set_alarm(rtt_wakeup, _entropy_timer_rtt_cb, arg);

        time = (uint8_t)timer_read(TIMER_DEV(PERIPH_TIMER_DEV));

        /* if counter value is odd, write upper 4 bit, else write lower */
        if (e_tim_arg->count_meas & 1) {
            shift = 4;
        }
        else {
            shift = 0;
        }

        e_tim_arg->pool[e_tim_arg->count_bytes] &= ~( 0xF << shift );
        e_tim_arg->pool[e_tim_arg->count_bytes] |= ( (0xF & time) << shift );

        /* decrement number of remaining iterations */
        e_tim_arg->count_meas--;

        DEBUG("%s: Value: %02i, Remaining: %02i, Byte no: %02i, %s 4 bits\n", __FUNCTION__, \
            (0xF & time), \
            e_tim_arg->count_meas, \
            e_tim_arg->count_bytes, \
            (shift) ? "upper" : "lower");

        if (shift) {
            /* increment bytes count every second iteration */
            e_tim_arg->count_bytes++;
        }

        /* reset the timer */
        timer_init(TIMER_DEV(PERIPH_TIMER_DEV), PERIPH_TIMER_FREQ, NULL, NULL);

    }
    else {
        /* clear any rtt alarm and stop gathering */
        rtt_clear_alarm();
        mutex_unlock(&lock);
        DEBUG("%s; finished gathering\n", __FUNCTION__);
    }
}

int entropy_timer_get(uint8_t *buf, size_t len)
{
    uint32_t rtt_now, rtt_wakeup;

    mutex_lock(&lock);

    /* one byte requires two cycles as we only pick 4 bits of one measure */
    e_tim.count_meas = len * 2;
    e_tim.pool = buf;

    /* initialize timers that are clocked from different sources */
    rtt_now = rtt_get_counter();
    rtt_wakeup = (rtt_now + RTT_TIMER_TICKS_WAIT) & RTT_MAX_VALUE;
    rtt_set_alarm(rtt_wakeup, _entropy_timer_rtt_cb, (void *)&e_tim);

    timer_init(TIMER_DEV(PERIPH_TIMER_DEV), PERIPH_TIMER_FREQ, NULL, NULL);

    /* wait for the gatering to complete */
    mutex_lock(&lock);

    return 0; // todo ass ressonable return vals
}
