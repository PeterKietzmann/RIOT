/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Test application for the timer based entropy
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>

#include "entropy_timer.h"

int main(void)
{
    puts("Start: Test entropy timer");
    uint8_t pool[16];

    entropy_timer_init();
    entropy_timer_get(pool, sizeof(pool));

    for (unsigned i = 0; i < sizeof(pool); i++) {
        printf("%i", pool[i]);
    }
    puts("");

    puts("End: Test finished");

    return 0;
}
