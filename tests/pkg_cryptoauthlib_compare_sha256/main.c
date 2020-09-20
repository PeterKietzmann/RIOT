/*
 * Copyright (C) 2019 HAW Hamburg
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
 * @brief       This test was written to compare the runtime of the RIOT software
 *              implementation and the CryptoAuth hardware implementation of SHA-256.
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "kernel_defines.h"
#include "periph/gpio.h"

#include "hashes/sha256.h"
#include "atca.h"
#include "atca_util.h"
#include "atca_params.h"

#include "shell.h"
#include "xtimer.h"
#include "periph/gpio.h"
#include "periph/i2c.h"

#define SHA256_HASH_SIZE (32)

#ifndef NUM_ITER
#define NUM_ITER 10
#endif

uint32_t start, stop;

uint8_t teststring[] =  "This is a teststring fore sha256";
uint8_t teststring2[] = "einealtedamegehthonigessenxxxxxx";
uint16_t test_string_size = (sizeof(teststring) - 1);   /* -1 to ignore \0 */

uint8_t expected[] =
{
0x65, 0x0C, 0x3A, 0xC7, 0xF9, 0x33, 0x17, 0xD3,
0x96, 0x31, 0xD3, 0xF5, 0xC5, 0x5B, 0x0A, 0x1E,
0x96, 0x68, 0x04, 0xE2, 0x73, 0xC3, 0x8F, 0x93,
0x9C, 0xB1, 0x45, 0x4D, 0xC2, 0x69, 0x7D, 0x20
};

uint8_t result[SHA256_HASH_SIZE];                       /* +3 to fit 1 byte length and 2 bytes checksum */

atca_sha256_ctx_t ctx;

static void ata_with_ctx_save(void)
{

    atca_sha256_ctx_t ctx;
    uint8_t res[SHA256_HASH_SIZE];
    uint8_t context[SHA_CONTEXT_MAX_SIZE];
    memset(context, 0, SHA_CONTEXT_MAX_SIZE);
    uint16_t context_size=sizeof(context);


    start = xtimer_now_usec();
#ifdef ATCA_MANUAL_ONOFF
    atecc_wake();
#endif


    for (int i = 0; i < NUM_ITER; i++) {
        // printf("LOOP #%i\n", i);
        if (atcab_hw_sha2_256_init(&ctx) != ATCA_SUCCESS) {
            puts("atcab_hw_sha2_256_initERROR");
        }
        if (atcab_sha_read_context(context, &context_size) != ATCA_SUCCESS) {
            puts("1atcab_sha_read_contextERROR");
        }

        if (atcab_sha_write_context(context, context_size) != ATCA_SUCCESS) {
        puts("atcab_sha_write_contextERROR");
        }
        if (atcab_hw_sha2_256_update(&ctx, teststring, sizeof(teststring)-1) != ATCA_SUCCESS) {
        puts("atcab_hw_sha2_256_updateERROR");
        }
        if (atcab_sha_read_context(context, &context_size) != ATCA_SUCCESS) {
        puts("2atcab_sha_read_contextERROR");
        }

        if (atcab_sha_write_context(context, context_size) != ATCA_SUCCESS) {
        puts("atcab_sha_write_contextERROR");
    }
        if (atcab_hw_sha2_256_finish(&ctx, res) != ATCA_SUCCESS) {
        puts("atcab_hw_sha2_256_finishERROR");
    }
    //     if (atcab_sha_read_context(context, &context_size) != ATCA_SUCCESS) {
    //     puts("3atcab_sha_read_contextERROR");
    // }
    }

#ifdef ATCA_MANUAL_ONOFF
    atecc_sleep();
#endif
    stop = xtimer_now_usec();
    printf("ata_with_ctx_save: %i Bytes int %"PRIu32 " us\n", (NUM_ITER*SHA256_HASH_SIZE), (stop-start));

    // if (!memcmp(expected, res, SHA256_HASH_SIZE) == 0) {
    //     puts("ata_with_ctx_save ERROR");
    // }
    // else {
    //     puts("ata_without_ctx_save alrido");
    // }
}

static void ata_without_ctx_save(void)
{
    atca_sha256_ctx_t ctx;
    uint8_t res[SHA256_HASH_SIZE];

    start = xtimer_now_usec();
#ifdef ATCA_MANUAL_ONOFF
    atecc_wake();
#endif


    for (int i = 0; i < NUM_ITER; i++) {
        // printf("LOOP #%i\n", i);
        if (atcab_hw_sha2_256_init(&ctx) != ATCA_SUCCESS) {
            puts("atcab_hw_sha2_256_initERROR");
        }
        if (atcab_hw_sha2_256_update(&ctx, teststring, sizeof(teststring)-1) != ATCA_SUCCESS) {
        puts("atcab_hw_sha2_256_updateERROR");
        }
        if (atcab_hw_sha2_256_finish(&ctx, res) != ATCA_SUCCESS) {
        puts("atcab_hw_sha2_256_finishERROR");
        }
    }

#ifdef ATCA_MANUAL_ONOFF
    atecc_sleep();
#endif

    stop = xtimer_now_usec();
    printf("ata_without_ctx_save: %i Bytes int %"PRIu32 " us\n", (NUM_ITER*SHA256_HASH_SIZE), (stop-start));
}


/**
 * Function to call RIOT software implementation of SHA-256
 */
// static int test_2_contexts(void)
// {
//     int ret;
//     sha256_context_t ctx, ctx2;
//     uint8_t res[SHA256_HASH_SIZE];
//     memset(res, 0, SHA256_HASH_SIZE);

//     // init first SW context
//     sha256_init(&ctx);
//     sha256_update(&ctx, (void *)teststring, sizeof(teststring)-1);

//     // init second SW context
//     sha256_init(&ctx2);
//     sha256_update(&ctx2, (void *)teststring2, sizeof(teststring2)-1);

//     // finalize first operation and compare with expected result
//     sha256_final(&ctx, res);

//     if (memcmp(expected, res, SHA256_HASH_SIZE) == 0) {
//         puts("RIOT SHA SW 2 contexts OK");
//     }
//     else {
//         puts("RIOT SHA SW 2 contexts SHICE");
//     }

//     // finalize second operation and keep digest in "res"
//     memset(res, 0, SHA256_HASH_SIZE);
//     sha256_final(&ctx2, res);

//     /* ATA */
//     atca_sha256_ctx_t actx, actx2;
//     uint8_t ares[SHA256_HASH_SIZE];
//     memset(ares, 0, SHA256_HASH_SIZE);
//     uint8_t context[SHA_CONTEXT_MAX_SIZE];
//     uint8_t context2[SHA_CONTEXT_MAX_SIZE];
//     uint16_t context_size=sizeof(context);
//     uint16_t context_size2 = sizeof(context2);

//     // init first ATA context
//     atcab_hw_sha2_256_init(&actx);
//     atcab_hw_sha2_256_update(&actx, teststring, sizeof(teststring)-1);

//     // save first context to reapply later
//     atcab_sha_read_context(context, &context_size);
//     // context_size = sizeof(actx.block);
//     // atcab_sha_read_context(actx.block, &context_size);

//     // printf("CONTEXT: \n");
//     // for(int i=0;i<context_size;i++) {
//     //     printf("%i ", context[i]);
//     // }
//     // printf("\n");

//     // init second ATA context
//     atcab_hw_sha2_256_init(&actx2);
//     atcab_hw_sha2_256_update(&actx2, teststring2, sizeof(teststring2)-1);

//     // save second context and apply first one
//     atcab_sha_read_context(context2, &context_size2);
//     atcab_sha_write_context(context, context_size);
//     // atcab_sha_write_context(actx.block, context_size);

//     // finalize first context and compare with expected value
//     ret = atcab_hw_sha2_256_finish(&actx, ares);

//     if (memcmp(expected, ares, SHA256_HASH_SIZE) == 0) {
//         puts("ATA SHA SW 2 contexts OK");
//     }
//     else {
//         puts("ATA SHA SW 2 contexts SHICE");
//     }

//     // repally second context
//     memset(ares, 0, SHA256_HASH_SIZE);
//     atcab_sha_write_context(context2, context_size);

//     // finalize second context and compare with RIOT results
//     ret = atcab_hw_sha2_256_finish(&actx2, ares);
//     printf("2 atcab_hw_sha2_256_finish returned: %x\n", ret);

//     if (memcmp(res, ares, SHA256_HASH_SIZE) == 0) {
//         puts("RIOT & ATA digest EQUAL");
//     }
//     else {
//         puts("RIOT & ATA digest SHOICE");
//     }

//     atcab_hw_sha2_256(teststring2, sizeof(teststring2) - 1, ares);
//     if (memcmp(res, ares, SHA256_HASH_SIZE) == 0)
//     {
//         puts("ATA convenience func OK");
//     }
//     else
//     {
//         puts("ATA convenience func SHOICE");
//     }
//     return 0;
// }


/**
 * Function to call CryptoAuth hardware implementation of SHA-256
 */
static int test_atca_sha(uint8_t *teststring, uint16_t len, uint8_t *expected,
                         uint8_t *result)
{
    atecc_wake();
    atcab_sha_start();
    atcab_sha_end(result, len, teststring);
    atecc_sleep();
    return memcmp(expected, result, SHA256_HASH_SIZE);
}

static int ata_sha_init(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    start = xtimer_now_usec();
    atcab_hw_sha2_256_init(&ctx);
    printf("atcab_hw_sha2_256_init: %"PRIu32"\n", xtimer_now_usec()-start);
    return 0;
}


static int ata_sha_update(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    start = xtimer_now_usec();
    atcab_hw_sha2_256_update(&ctx, teststring, test_string_size);
    printf("atcab_hw_sha2_256_update: %"PRIu32"\n", xtimer_now_usec()-start);
    return 0;
}

static int ata_sha_finish(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    start = xtimer_now_usec();
    atcab_hw_sha2_256_finish(&ctx, result);
    printf("atcab_hw_sha2_256_finish: %"PRIu32"\n", xtimer_now_usec()-start);
    return 0;
}

static int ata_wake(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    start = xtimer_now_usec();
    atecc_wake();
    printf("atecc_wake: %"PRIu32";\n", xtimer_now_usec()-start);
    return 0;
}

static int ata_serial(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    uint8_t sn[ATCA_SERIAL_NUM_SIZE];
    ATCA_STATUS status;

    if ((status = atcab_read_serial_number(sn)) != ATCA_SUCCESS)
    {
        puts("error reading SN");
        return -1;
    }
    printf("SERIAl NO: ");
    for (unsigned i=0;i<sizeof(sn);i++) {
        printf("0x%x ", sn[i]);
    }
    printf("\n");
    return 0;
}

static int ata_idle(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    start = xtimer_now_usec();
    atecc_idle();
    printf("atecc_idle: %"PRIu32";\n", xtimer_now_usec()-start);
    return 0;
}
static int ata_sleep(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    start = xtimer_now_usec();
    atecc_sleep();
    printf("atecc_sleep: %"PRIu32";\n", xtimer_now_usec()-start);

    return 0;
}
static const shell_command_t shell_commands[] = {
    { "asi", "ata sha init", ata_sha_init },
    { "asu", "ata sha update", ata_sha_update },
    { "asf", "ata sha finish", ata_sha_finish },
    { "awake", "ata wake up", ata_wake },
    { "aidle", "ata idle", ata_idle },
    { "asleep", "ata sleep", ata_sleep },
    { "aserial", "ata serial", ata_serial },
    { NULL, NULL, NULL }
};


const uint8_t g_aes_keys[4][16] = {
    // { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C },
    {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c},
    { 0x6B, 0xE1, 0x63, 0xD4, 0x2B, 0x62, 0x3E, 0x70, 0xD1, 0x64, 0xFA, 0x14, 0x5D, 0xB1, 0xD4, 0x63 },
    // { 0x70, 0x58, 0x71, 0x0B, 0x58, 0xE1, 0xE6, 0x65, 0xD3, 0xD2, 0xF5, 0xB4, 0x65, 0x17, 0x64, 0x03 },
    // { 0x11, 0x44, 0x43, 0xFA, 0x8E, 0x96, 0x14, 0x84, 0x5E, 0xC7, 0x29, 0x6C, 0xD1, 0x3B, 0xC9, 0xDC }
};

static uint8_t __attribute__((aligned)) ECB_PLAIN[] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a
};
static uint8_t __attribute__((aligned))ECB_CIPHER[] = {
    0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
    0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97
};
static uint8_t CBC_IV[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};
static uint8_t __attribute__((aligned)) CBC_PLAIN[] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
        0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
        0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51
};
static uint8_t __attribute__((aligned)) CBC_CIPHER[] = {
    0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
    0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
    0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee,
    0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2
};

static uint8_t HMAC_KEY[] = {
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b
};

static uint8_t HMAC_INPUT[] = {
    0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
    0x61, 0x20, 0x74, 0x65, 0x73, 0x74, 0x73, 0x74,
    0x72, 0x69, 0x6e, 0x67, 0x20, 0x66, 0x6f, 0x72,
    0x20, 0x68, 0x6d, 0x61, 0x63, 0x32, 0x35, 0x36,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00q
};
static uint8_t EXPECTED_RESULT[] = {
    0x39, 0x38, 0x66, 0x75, 0xdc, 0xc9, 0xe1, 0x86,
    0x58, 0xac, 0xfe, 0x34, 0x05, 0x79, 0xe5, 0x1b,
    0x20, 0x02, 0x8d, 0xc6, 0x3c, 0x70, 0xaf, 0x80,
    0xe5, 0x2d, 0xe4, 0x22, 0x7a, 0x41, 0x0c, 0x70
};

int main(void)
{

    memset(result, 0, SHA256_HASH_SIZE);                    /* alles in result auf 0 setzen */

#ifdef NO_I2C_RECONF
    puts("SLOW I2C");
#else
    puts("FAST I2C");
#endif

#ifdef MODULE_PERIPH_I2C_RECONFIGURE
    puts("GPIO WAKEUP");
#else
    puts("I2C WAKEUP");
#endif

    ATCA_STATUS status;
    uint8_t encrypted_data_out[16];
    uint8_t decrypted_data_out[16];

    // ENC
    atcab_nonce_load(NONCE_MODE_TARGET_TEMPKEY, g_aes_keys[0], 32);

    for (unsigned data_block = 0; data_block < sizeof(ECB_PLAIN) / AES_DATA_SIZE; data_block++)
    {
        status = atcab_aes_encrypt(ATCA_TEMPKEY_KEYID, 0, &ECB_PLAIN[data_block * AES_DATA_SIZE], encrypted_data_out);
        if(status != ATCA_SUCCESS) {
            puts("ERROR");
        }
    }
    if(memcmp(ECB_CIPHER, encrypted_data_out, AES_DATA_SIZE) !=0) {
        puts("MISMATCH");
        for (int i=0;i<AES_DATA_SIZE;i++) {
            printf("%x \n", encrypted_data_out[i]);
        }
    }

    // DEC
    status = atcab_nonce_load(NONCE_MODE_TARGET_TEMPKEY, g_aes_keys[0], 64);

    for (unsigned data_block = 0; data_block < sizeof(encrypted_data_out) / AES_DATA_SIZE; data_block++)
    {
        status = atcab_aes_decrypt(ATCA_TEMPKEY_KEYID, 0, &encrypted_data_out[data_block * AES_DATA_SIZE], decrypted_data_out);
        if(status != ATCA_SUCCESS) {
            puts("ERROR");
        }
    }
    if(memcmp(ECB_PLAIN, decrypted_data_out, AES_DATA_SIZE) !=0) {
        puts("MISMATCH");
        for (int i=0;i<AES_DATA_SIZE;i++) {
            printf("%x \n", encrypted_data_out[i]);
        }
    }

    // ENC CBC
    atca_aes_cbc_ctx_t ctx;
    uint8_t cbc_encrypted_data_out[32];
    uint8_t cbc_decrypted_data_out[32];

    status = atcab_nonce_load(NONCE_MODE_TARGET_TEMPKEY, g_aes_keys[0], 32);
    status = atcab_aes_cbc_init(&ctx, ATCA_TEMPKEY_KEYID, 0, CBC_IV);

    for (unsigned data_block = 0; data_block < sizeof(CBC_PLAIN) / AES_DATA_SIZE; data_block++)
    {
        int idx=data_block * AES_DATA_SIZE;
        status = atcab_aes_cbc_encrypt_block(&ctx, &CBC_PLAIN[idx], &cbc_encrypted_data_out[idx]);
    }
    if(memcmp(CBC_CIPHER, cbc_encrypted_data_out, sizeof(CBC_CIPHER)) !=0) {
        puts("CBC ENC MISMATCH");
        for (int i=0;i<AES_DATA_SIZE;i++) {
            printf("%x %x\n", CBC_CIPHER[i], cbc_encrypted_data_out[i]);
        }
    }

    // DEC CBC
    status = atcab_nonce_load(NONCE_MODE_TARGET_TEMPKEY, g_aes_keys[0], 32);
    status = atcab_aes_cbc_init(&ctx, ATCA_TEMPKEY_KEYID, 0, CBC_IV);

    for (unsigned data_block = 0; data_block < sizeof(CBC_CIPHER) / AES_DATA_SIZE; data_block++)
    {
        int idx=data_block * AES_DATA_SIZE;
        status = atcab_aes_cbc_decrypt_block(&ctx, &cbc_encrypted_data_out[idx], &cbc_decrypted_data_out[idx]);
    }
    if(memcmp(CBC_PLAIN, cbc_decrypted_data_out, sizeof(CBC_PLAIN)) !=0) {
        puts("CBC DEC MISMATCH");
        for (int i=0;i<AES_DATA_SIZE;i++) {
            printf("%x %x\n", CBC_PLAIN[i], cbc_decrypted_data_out[i]);
        }
    }

    // HMAC SHA256
    uint8_t digest[32];
    atca_hmac_sha256_ctx_t ctx_hmac;

    status = atcab_nonce_load(NONCE_MODE_TARGET_TEMPKEY, HMAC_KEY, 64);
    if(status != ATCA_SUCCESS) {
        puts("ERROR 0");
    }

    status = atcab_sha_hmac_init(&ctx_hmac, ATCA_TEMPKEY_KEYID);
    if(status != ATCA_SUCCESS) {
        puts("ERROR 1");
    }
    status = atcab_sha_hmac_update(&ctx_hmac, HMAC_INPUT, sizeof(HMAC_INPUT));
    if(status != ATCA_SUCCESS) {
        puts("ERROR 2");
    }
    status = atcab_sha_hmac_finish(&ctx_hmac, digest, SHA_MODE_TARGET_OUT_ONLY);
    if(status != ATCA_SUCCESS) {
        puts("ERROR 3");
    }

    if(memcmp(EXPECTED_RESULT, digest, sizeof(digest)) != 0)
    {
        puts("DIGEST MISMATCH");
        for (unsigned i=0;i<sizeof(digest);i++) {
            printf("%x %x\n", EXPECTED_RESULT[i], digest[i]);
        }
    }




    ATCAIfaceCfg *cfg = (ATCAIfaceCfg *)&atca_params[I2C_DEV(0)];
    if (i2c_config[cfg->atcai2c.bus].speed == I2C_SPEED_NORMAL) {
        puts("I2C_SPEED_NORMAL");
    }
    if (i2c_config[cfg->atcai2c.bus].speed == I2C_SPEED_FAST) {
        puts("I2C_SPEED_FAST");
    }

#ifdef ATCA_MANUAL_ONOFF
    // sleep initially
    atecc_sleep();
    puts("ATCA_MANUAL_ON");
#else
    puts("ATCA_MANUAL_OFF");
#endif

    if (test_atca_sha(teststring, test_string_size, expected, result) == 0) {
        printf("ATCA SHA256: Success\n");
    }
    else {
        printf("ATCA SHA256: Failure.\n");
    }
    ata_with_ctx_save();
    ata_without_ctx_save();

    // test_2_contexts();


     /* define buffer to be used by the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    /* define own shell commands */
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
