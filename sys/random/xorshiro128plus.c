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
 * @brief   Xoroshiro128 Plus random number generator implementation
 *
 * @author Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 * @}
 */

#include <stdint.h>
#include <string.h>

#include "random.h"
#include "log.h"

static uint64_t _state128[2];

static inline uint64_t rotl (uint64_t a, int w) {
   return (a << w) | (a >> (64 - w));
}

static uint64_t splitmix64(uint64_t *seed)
{
    uint64_t z = (*seed += (uint64_t)(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * (uint64_t)(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * (uint64_t)(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

static void _init_weak(uint32_t init_key[], int key_length)
{
    uint64_t tmp = 0;

    tmp = (uint64_t)(init_key[0]);

    if (key_length > 1) {
        tmp |= (uint64_t)(init_key[1]);
    }
    _state128[0] ^= splitmix64(&tmp);

    if (key_length > 2) {
        tmp = (uint64_t)(init_key[2]);
    }
    _state128[1] ^= splitmix64(&tmp);

    if (_state128[1] == 0 && _state128[0] == 0) {
        _state128[0] = 1;
    }
}

uint64_t xoroshiro128plus(uint64_t *state)
{
    const uint64_t s0 = state[0];
    uint64_t s1 = state[1];
    const uint64_t result = s0 + s1;

   s1 ^= s0;
   state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14);
   state[1] = rotl(s1, 36);

   return result;
}

uint32_t random_uint32(void)
{
    static uint64_t buf;
    static int jmp = 0;

    if (!jmp) { /*  TODO: reset in init? */
        jmp = 1;
        buf = xoroshiro128plus(_state128);
        return (buf & 0xffffffff);
    }
    jmp = 0;
    return ((buf >> 32) & 0xffffffff);
}

void random_init(uint32_t val)
{
    random_init_by_array(&val, 1);
}

void random_init_by_array(uint32_t init_key[], int key_length)
{
    memset(_state128, 0, sizeof(_state128));

    if (key_length < 4) { /*TODO check for 0 and negative*/
        LOG_WARNING("xoroshiro128plus not seeded properly.\n");

        _init_weak(init_key, key_length);
    }
    else {
        for (int i = 0; i < (int) key_length; i++) {
            _state128[i % 4] ^= init_key[i];
        }
    }
}