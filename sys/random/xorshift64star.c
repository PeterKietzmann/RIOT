/**
 * Copyright (C) 2020 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 * Code taken from Wikipedia (https://en.wikipedia.org/wiki/Xorshift)
 */

 /**
 * @ingroup sys_random
 * @{
 * @file
 *
 * @brief   Xorshift64* random number generator implementation
 *
 * @author Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 * @}
 */

#include <stdint.h>
#include <string.h>

#include "random.h"
#include "log.h"

static uint64_t splitmix64(uint64_t *seed)
{
    uint64_t z = (*seed += (uint64_t)(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * (uint64_t)(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * (uint64_t)(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

uint64_t xorshift64star(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * (uint64_t)0x2545F4914F6CDD1D;
}

static uint64_t _state64;

uint32_t random_uint32(void)
{
    static uint64_t buf;
    static int jmp = 0;

    if (!jmp) {
        jmp = 1;
        buf = xorshift64star(&_state64);
        return (buf & 0xffffffff);
    }
    jmp = 0;
    return ((buf >> 32) & 0xffffffff);
}

void random_init(uint32_t val)
{
    LOG_WARNING("xorshift64star not seeded properly.\n");

    uint64_t tmp;

    _state64 = 0;
    tmp = (uint64_t)(val);

    _state64 ^= splitmix64(&tmp);
}

void random_init_by_array(uint32_t init_key[], int key_length)
{
    if (key_length < 2) { /*TODO check for 0 and negative*/
        random_init(*init_key);
    }
    else {
        memcpy(&_state64, init_key, 2);
    }
}