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
    uint8_t pool[4];

    for (unsigned j = 0; j < 500; j++) {
        puts("Start: Test entropy timer");
        entropy_timer_get(pool, sizeof(pool));
        printf("Success: Data for puf_sram_seed: [0x");
        for (unsigned i = 0; i < sizeof(pool); i++) {
            printf("%02x", pool[i]);
        }
        puts("]");

        puts("End: Test finished");
    }
    return 0;
}
