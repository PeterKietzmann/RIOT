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

#define SHA256_HASH_SIZE (32)

uint32_t start;

uint8_t teststring[] = "chili cheese fries";
uint8_t teststring2[] = "einealtedamegehthonigessen";
uint16_t test_string_size = (sizeof(teststring) - 1);   /* -1 to ignore \0 */

uint8_t expected[] =
{ 0x36, 0x46, 0xEF, 0xD6, 0x27, 0x6C, 0x0D, 0xCB, 0x4B, 0x07, 0x73, 0x41,
    0x88, 0xF4, 0x17, 0xB4, 0x38, 0xAA, 0xCF, 0xC6, 0xAE, 0xEF, 0xFA, 0xBE,
    0xF3, 0xA8, 0x5D, 0x67, 0x42, 0x0D, 0xFE, 0xE5 };

uint8_t result[SHA256_HASH_SIZE];                       /* +3 to fit 1 byte length and 2 bytes checksum */

atca_sha256_ctx_t ctx;


/**
 * Function to call RIOT software implementation of SHA-256
 */
static int test_2_contexts(void)
{
    int ret;
    sha256_context_t ctx, ctx2;
    uint8_t res[SHA256_HASH_SIZE];
    memset(res, 0, SHA256_HASH_SIZE);

    // init first SW context
    sha256_init(&ctx);
    sha256_update(&ctx, (void *)teststring, sizeof(teststring)-1);

    // init second SW context
    sha256_init(&ctx2);
    sha256_update(&ctx2, (void *)teststring2, sizeof(teststring2)-1);

    // finalize first operation and compare with expected result
    sha256_final(&ctx, res);

    if(memcmp(expected, res, SHA256_HASH_SIZE) == 0) {
        puts("RIOT SHA SW 2 contexts OK");
    }
    else {
        puts("RIOT SHA SW 2 contexts SHICE");
    }

    // finalize second operation and keep digest in "res"
    memset(res, 0, SHA256_HASH_SIZE);
    sha256_final(&ctx2, res);

    /* ATA */
    atca_sha256_ctx_t actx, actx2;
    uint8_t ares[SHA256_HASH_SIZE];
    memset(ares, 0, SHA256_HASH_SIZE);
    uint8_t context[SHA_CONTEXT_MAX_SIZE];
    uint8_t context2[SHA_CONTEXT_MAX_SIZE];
    uint16_t context_size=sizeof(context);
    uint16_t context_size2 = sizeof(context2);

    // init first ATA context
    atcab_hw_sha2_256_init(&actx);
    atcab_hw_sha2_256_update(&actx, teststring, sizeof(teststring)-1);

    // save first context to reapply later
    atcab_sha_read_context(context, &context_size);
    // context_size = sizeof(actx.block);
    // atcab_sha_read_context(actx.block, &context_size);

    // printf("CONTEXT: \n");
    // for(int i=0;i<context_size;i++){
    //     printf("%i ", context[i]);
    // }
    // printf("\n");

    // init second ATA context
    atcab_hw_sha2_256_init(&actx2);
    atcab_hw_sha2_256_update(&actx2, teststring2, sizeof(teststring2)-1);

    // save second context and apply first one
    atcab_sha_read_context(context2, &context_size2);
    atcab_sha_write_context(context, context_size);
    // atcab_sha_write_context(actx.block, context_size);

    // finalize first context and compare with expected value
    ret = atcab_hw_sha2_256_finish(&actx, ares);

    if(memcmp(expected, ares, SHA256_HASH_SIZE) == 0) {
        puts("ATA SHA SW 2 contexts OK");
    }
    else {
        puts("ATA SHA SW 2 contexts SHICE");
    }

    // repally second context
    memset(ares, 0, SHA256_HASH_SIZE);
    atcab_sha_write_context(context2, context_size);

    // finalize second context and compare with RIOT results
    ret = atcab_hw_sha2_256_finish(&actx2, ares);
    printf("2 atcab_hw_sha2_256_finish returned: %x\n", ret);

    if(memcmp(res, ares, SHA256_HASH_SIZE) == 0) {
        puts("RIOT & ATA digest EQUAL");
    }
    else {
        puts("RIOT & ATA digest SHOICE");
    }

    atcab_hw_sha2_256(teststring, sizeof(teststring) - 1, ares);
    if (memcmp(res, ares, SHA256_HASH_SIZE) == 0)
    {
        puts("ATA convenience func OK");
    }
    else
    {
        puts("ATA convenience func SHOICE");
    }
    return 0;
}

/**
 * Function to call RIOT software implementation of SHA-256
 */
static int test_riot_sha256(uint8_t *teststring, uint16_t len,
                            uint8_t *expected,
                            uint8_t *result)
{

    sha256_context_t ctx;
    start = xtimer_now_usec();
    sha256_init(&ctx);
    printf("sha256_init: %"PRIu32"\n", xtimer_now_usec()-start);

    start = xtimer_now_usec();
    sha256_update(&ctx, (void *)teststring, len);
    printf("sha256_update: %"PRIu32"\n", xtimer_now_usec()-start);

    start = xtimer_now_usec();
    sha256_final(&ctx, result);
    printf("sha256_final: %"PRIu32"\n", xtimer_now_usec()-start);

    return memcmp(expected, result, SHA256_HASH_SIZE);
}

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
    int ret = atcab_wakeup();
    // atcab_idle();// prevents auto going to sleep
    printf("atcab_wakeup: %"PRIu32"; ret: %i\n", xtimer_now_usec()-start, ret);
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
    for (unsigned i=0;i<sizeof(sn);i++){
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
    int ret = atcab_idle();
    printf("atcab_idle: %"PRIu32"; ret: %i\n", xtimer_now_usec()-start, ret);
    return 0;
}
static int ata_sleep(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    start = xtimer_now_usec();
    int ret = atcab_sleep();
    printf("atcab_sleep: %"PRIu32"; ret: %i\n", xtimer_now_usec()-start, ret);

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

int main(void)
{

    memset(result, 0, SHA256_HASH_SIZE);                    /* alles in result auf 0 setzen */

    test_2_contexts();

    if (test_riot_sha256(teststring, test_string_size, expected, result) == 0) {
        printf("RIOT SHA256: Success\n");
    }
    else {
        printf("RIOT SHA256: Failure.\n");
    }
    atca_delay_us(10);
    memset(result, 0, SHA256_HASH_SIZE);

    if (test_atca_sha(teststring, test_string_size, expected, result) == 0) {
        printf("ATCA SHA256: Success\n");
    }
    else {
        printf("ATCA SHA256: Failure.\n");
    }

     /* define buffer to be used by the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    /* define own shell commands */
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
