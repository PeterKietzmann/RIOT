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
 * @brief       This application compares SOFTWARE performance metrics of different cryptographic libraries
 *
 * @author      PeterKietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include "xtimer.h"

#if TEST_RIOT
#include "crypto/aes.h"
#endif

#if TEST_RELIC
#include "relic.h"
#include "../src/bc/rijndael-api-fst.h"
#endif

#if TEST_TINYCRYPT
#include "tinycrypt/aes.h"
#include "tinycrypt/constants.h"
#endif

#if TEST_WOLFSSL
#include "wolfssl/wolfcrypt/aes.h"
#endif

#if TEST_CIFRA
#include "aes.h"
#endif

static uint8_t TEST_1_KEY[] = {
    0x23, 0xA0, 0x18, 0x53, 0xFA, 0xB3, 0x89, 0x23,
    0x65, 0x89, 0x2A, 0xBC, 0x43, 0x99, 0xCC, 0x00
};

static uint8_t TEST_1_INP[] = {
    0x11, 0x53, 0x81, 0xE2, 0x5F, 0x33, 0xE7, 0x41,
    0xBB, 0x12, 0x67, 0x38, 0xE9, 0x12, 0x54, 0x23
};

static uint8_t TEST_1_ENC[] = {
    0xD7, 0x9A, 0x54, 0x0E, 0x61, 0x33, 0x03, 0x72,
    0x59, 0x0f, 0x87, 0x91, 0xEF, 0xB0, 0xF8, 0x16
};

uint32_t start, stop;

void _start_tim(void) {
    start = xtimer_now_usec();
}

void _stop_tim(void) {
    stop = xtimer_now_usec();
    printf("%"PRIu32"\n", (stop - start));
    start = 0;
}

int main(void)
{
    uint8_t data_enc[16]={0};
    uint8_t data_dec[16]={0};
    (void)TEST_1_ENC;


#if TEST_RIOT
    puts("RIOT");

    cipher_context_t ctx;

    int err;
    err = aes_init(&ctx, TEST_1_KEY, sizeof(TEST_1_KEY));
    if (err <= 0) {
        puts("error init");
    }
    err = aes_encrypt(&ctx, TEST_1_INP, data_enc);
    if (err < 0) {
        puts("error enc\n");
    }

    err = aes_decrypt(&ctx, data_enc, data_dec);
    if (err < 0) {
        puts("error dec");
    }

    if (memcmp(TEST_1_INP, data_dec, sizeof(data_dec)) != 0) {
        printf("Error decrypting\n");
    }
#endif

#if TEST_RELIC
    puts("RELIC");

    cipherInstance ctx;
    keyInstance key_inst;

    core_init();

    _start_tim();
	if (cipherInit(&ctx, MODE_ECB, NULL) != TRUE) {
		puts("0. ERR_NO_VALID");
	}
    _stop_tim();
    _start_tim();
	if (makeKey2(&key_inst, DIR_ENCRYPT, 8 * sizeof(TEST_1_KEY), (char *)TEST_1_KEY) != TRUE) {
		puts("makeKey2 enc error");
	}
    if (blockEncrypt(&ctx, &key_inst,
		TEST_1_INP, 8 * sizeof(TEST_1_INP), data_enc) <=0) {
        puts("1. ERR_NO_VALID");
    }
    _stop_tim();
    _start_tim();
	if (makeKey2(&key_inst, DIR_DECRYPT, 8 * sizeof(TEST_1_KEY), (char *)TEST_1_KEY) != TRUE) {
		puts("makeKey2 dec error");
	}
    if (blockDecrypt(&ctx, &key_inst,
        data_enc, 8 * sizeof(data_enc), data_dec) <=0) {
        puts("2. ERR_NO_VALID");
    }
    _stop_tim();

    if (memcmp(TEST_1_INP, data_dec, sizeof(data_enc)) != 0) {
        puts("RELIC AES HASHES MISMATCH");
    }
    // for(unsigned i=0;i<sizeof(data_dec);i++) {
    //     printf("%02x %02x %02x %02x\n", TEST_1_ENC[i], data_enc[i], TEST_1_INP[i],data_dec[i]);
    // }
    core_clean();
#endif

#if TEST_TINYCRYPT
    puts("TINYCRYPT");

    struct tc_aes_key_sched_struct ctx;

    // fake. there is no init in this API
    printf("0\n");

    _start_tim();
    if (tc_aes128_set_encrypt_key(&ctx, TEST_1_KEY) != TC_CRYPTO_SUCCESS) {
        puts("0. ERR_NO_VALID");
    }
    if (tc_aes_encrypt(data_enc, TEST_1_INP, &ctx) != TC_CRYPTO_SUCCESS) {
        puts("1. ERR_NO_VALID");
    }
    _stop_tim();
    _start_tim();
    if (tc_aes128_set_decrypt_key(&ctx, TEST_1_KEY) != TC_CRYPTO_SUCCESS) {
        puts("2. ERR_NO_VALID");
    }
    if (tc_aes_decrypt(data_dec, data_enc, &ctx) != TC_CRYPTO_SUCCESS) {
        puts("3. ERR_NO_VALID");
    }
    _stop_tim();
    if (memcmp(TEST_1_INP, data_dec, sizeof(data_dec)) != 0) {
        puts("TINYCRYPT AES MISMATCH");
    }
#endif

#if TEST_WOLFSSL
    puts("WOLFSSL");

    Aes ctx;

    _start_tim();
    if (wc_AesInit(&ctx, NULL, INVALID_DEVID) != 0) {
        puts("0. ERR_NO_VALID");
    }
    if (wc_AesSetKey(&ctx, TEST_1_KEY, sizeof(TEST_1_KEY), NULL, AES_ENCRYPTION) != 0) {
        puts("1. ERR_NO_VALID");
    }
    _stop_tim();
    _start_tim();
    wc_AesEncryptDirect(&ctx, data_enc, TEST_1_INP);
    _stop_tim();

    _start_tim();
    if (wc_AesSetKey(&ctx, TEST_1_KEY, sizeof(TEST_1_KEY), NULL, AES_DECRYPTION) != 0) {
        puts("2. ERR_NO_VALID");
    }
    memset(data_dec, 0, sizeof(data_dec));
    wc_AesDecryptDirect(&ctx, data_dec, data_enc);
    _stop_tim();

    if (memcmp(TEST_1_INP, data_dec, sizeof(data_dec)) != 0) {
        puts("WOLFSSL AES MISMATCH");
    }
#endif

#if TEST_CIFRA
    puts("CIFRA");

    cf_aes_context ctx;

    _start_tim();
    cf_aes_init(&ctx, TEST_1_KEY, sizeof(TEST_1_KEY));
    _stop_tim();
    _start_tim();
    cf_aes_encrypt(&ctx, TEST_1_INP, data_enc);
    _stop_tim();
    _start_tim();
    cf_aes_decrypt(&ctx, data_enc, data_dec);
    _stop_tim();
    if (memcmp(TEST_1_INP, data_dec, sizeof(data_dec)) != 0) {
        puts("CIFRA AES MISMATCH");
    }
#endif
    puts("DONE");

    // for(unsigned i=0;i<sizeof(sha256_hash);i++) {
    //     printf("%x %x \n", expected_result_sha256[i], sha256_hash[i]);
    // }

    return 0;
}
