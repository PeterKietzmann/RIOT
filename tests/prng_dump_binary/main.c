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
#include "xtimer.h"
#include "ps.h"
#include "periph/gpio.h"

#ifdef MODULE_PERIPH_HWRNG
#include "periph/hwrng.h"
#endif

#ifdef CPU_FAM_STM32F4
#define PIN0 GPIO_PIN(1,  5)
#define PIN1 GPIO_PIN(1,  3)
#endif

#ifdef CPU_FAM_SAMD21
#define PIN0 GPIO_PIN(0, 16)
#define PIN1 GPIO_PIN(0, 17)
#endif

int main(void)
{
#ifdef TOGGLE_PIN_MODE
    puts("TOGLE_PIN_MODE");
    gpio_init(PIN0, GPIO_OUT);
    gpio_init(PIN1, GPIO_OUT);

    gpio_clear(PIN0);
    gpio_set(PIN0);
    xtimer_usleep(10);
    gpio_clear(PIN0);
    xtimer_usleep(10);
    gpio_set(PIN0);
#endif

#ifdef MODULE_PERIPH_HWRNG
    hwrng_init();
#else
    random_init(534571505);
#endif

#ifdef TOGGLE_PIN_MODE
    gpio_clear(PIN0);
#endif

#ifndef MEM_USED
    while(1) {
#endif
        uint32_t rand_val=0;

#ifdef TOGGLE_PIN_MODE
        gpio_set(PIN1);
#endif

#ifdef MODULE_PERIPH_HWRNG
        hwrng_read(&rand_val, 4);
#else
        rand_val = random_uint32();
#endif

#ifdef TOGGLE_PIN_MODE
        gpio_clear(PIN1);
        (void)rand_val;
#else
        for (unsigned i =0; i< sizeof(uint32_t); i++) {
            printf("%c", (char)( (rand_val >> (8*i) & 0xff)) );
        }
#endif

#ifndef MEM_USED
    }
#else

        ps();
#if MODULE_PRNG_FORTUNA
        puts("Fortuna PRNG.\n");
#elif MODULE_PRNG_MERSENNE
        puts("Mersenne Twister PRNG.\n");
#elif MODULE_PRNG_MINSTD
        puts("Park & Miller Minimal Standard PRNG.\n");
#elif MODULE_PRNG_MUSL_LCG
        puts("Musl C PRNG.\n");
#elif MODULE_PRNG_SHA1PRNG
        puts("SHA1 PRNG.\n");
#elif MODULE_PRNG_TINYMT32
        puts("Tiny Mersenne Twister PRNG.\n");
#elif MODULE_PRNG_XORSHIFT
        puts("XOR Shift PRNG.\n");
#else
        puts("unknown PRNG.\n");
#endif
#endif

    return 0;
}
