/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Low-level random number generator driver test
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "xtimer.h"
#include "periph/hwrng.h"

#define LIMIT       (3130000U)

int main(void)
{
    uint8_t buf[4];

    puts("\nHWRNG peripheral driver test\n");
    printf("This test will print from 1 to %u random bytes about every "
           "second\n\n", LIMIT);

        /* create random numbers */
        for (unsigned i = 1; i <= LIMIT; i++) {
            /* zero out buffer */
            memset(buf, 0, sizeof(buf));
            hwrng_read(buf, 4);
            for (unsigned j = 0; j < 4; j++) {
                printf("%02x", (unsigned char)buf[j]);
            }
            printf("\n");
        }

    return 0;
}
