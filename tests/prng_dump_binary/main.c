/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       PRNG binary output
 *
 * @author      Peter Kietzmann
 *
 * @}
 */

#include <stdio.h>
#include "random.h"

#ifdef MODULE_PERIPH_HWRNG
#include "periph/hwrng.h"
#endif


int main(void)
{
#ifdef MODULE_PERIPH_HWRNG
    hwrng_init();
#else
    random_init(534571505);
#endif

    while(1) {
        uint32_t rand_val=0;
#ifdef MODULE_PERIPH_HWRNG
        hwrng_read(&rand_val, 4);
#else
        rand_val = random_uint32();
#endif
        for (unsigned i =0; i< sizeof(uint32_t); i++) {
            printf("%c", (char)( (rand_val >> (8*i) & 0xff)) );
        }
    }

    return 0;
}
