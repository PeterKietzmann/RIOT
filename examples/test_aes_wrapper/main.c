/*
 * Copyright (C) 2020 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       x
 *
 * @author      y
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "crypto/aes.h"
#include "crypto/ciphers.h"
#include "xtimer.h"

/* AES Test */
static uint8_t TEST_0_KEY[] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
};
static uint8_t TEST_0_INP[] = {
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
};
static uint8_t TEST_0_ENC[] = {
    0x37, 0x29, 0xa3, 0x6c, 0xaf, 0xe9, 0x84, 0xff,
    0x46, 0x22, 0x70, 0x42, 0xee, 0x24, 0x83, 0xf6
};

/* Timer variables */
uint32_t start, stop, t_diff;

static void aes_test(void)
{
    int err;
    uint8_t data[AES_BLOCK_SIZE];
    memset(data, 0, AES_BLOCK_SIZE);
#ifdef PLAIN_AES
    puts("PLAIN_AES");
    cipher_context_t c_ctx;
    err = aes_init(&c_ctx, TEST_0_KEY, AES_KEY_SIZE);
#else
    puts("CIPHER");
    cipher_context_t c_ctx;
    err = cipher_init(&c_ctx, CIPHER_AES_128, TEST_0_KEY, AES_KEY_SIZE);
#endif
    if (err < 1) {
        printf("AES Init failed: %d\n", err);
    }

    start = xtimer_now_usec();
#ifdef PLAIN_AES
    if (aes_encrypt(&c_ctx, TEST_0_INP, data)) {
#else
    if (cipher_encrypt(&c_ctx, CIPHER_AES_128, TEST_0_INP, data)) {
#endif
        stop = xtimer_now_usec();
        t_diff = stop - start;
        printf("AES Encrypt time: %ld us\n", t_diff);
        if (!memcmp(data, TEST_0_ENC, AES_BLOCK_SIZE)) {
            printf("AES encryption successful\n");
        }
        else
        {
            printf("AES encryption failed\n");
            for (int i = 0; i < 16; i++) {
                printf("%02x ", data[i]);
            }
            printf("\n");
        }
    }
    memset(data, 0, AES_BLOCK_SIZE);
    start = xtimer_now_usec();
#if PLAIN_AES
    if (aes_decrypt(&c_ctx, TEST_0_ENC, data)) {
#else
    if (cipher_decrypt(&c_ctx, CIPHER_AES_128, TEST_0_ENC, data)) {
#endif
        stop = xtimer_now_usec();
        t_diff = stop - start;
        printf("AES Decrypt time: %ld us\n", t_diff);
        if (!memcmp(data, TEST_0_INP, AES_BLOCK_SIZE)) {
            printf("AES decryption successful\n");
        }
        else
        {
            printf("AES decryption failed\n");
            for (int i = 0; i < 16; i++) {
                printf("%02x ", data[i]);
            }
            printf("\n");
        }
    }
}

static uint8_t TEST_1_COUNTER[16] = {
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};
static uint8_t TEST_1_PLAIN[] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
    0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
    0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
    0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
};
static uint8_t TEST_1_PLAIN_LEN = 64;

static uint8_t TEST_1_KEY[] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
static uint8_t TEST_1_KEY_LEN = 16;

int main(void)
{
    aes_test();

    uint8_t ctr[16];
    uint8_t data[64];
    cipher_context_t ctx;

    memcpy(ctr, TEST_1_COUNTER, 16);

    aes_init(&ctx, TEST_1_KEY, TEST_1_KEY_LEN);
    aes_encrypt_ctr(&ctx, ctr, 0, TEST_1_PLAIN, TEST_1_PLAIN_LEN, data);

    printf("\nTEST: ");
    for(unsigned i=0;i<sizeof(data);i++) {
        printf("%i ", data[i]);
    }
    printf("\n");

    return 0;
}
