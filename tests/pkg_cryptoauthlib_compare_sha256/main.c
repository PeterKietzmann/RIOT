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
#include "atca_params.h"

#define SHA256_HASH_SIZE (32)

void atecc_wake(void)
{
    ATCAIfaceCfg *cfg = (ATCAIfaceCfg *)&atca_params[I2C_DEV(0)];
    uint8_t data[4] = { 0 };

#if IS_USED(MODULE_PERIPH_I2C_RECONFIGURE)
    /* switch I2C peripheral to GPIO function */
    i2c_deinit_pins(cfg->atcai2c.bus);
    gpio_init(i2c_pin_sda(cfg->atcai2c.bus), GPIO_OUT);

    /* send wake pulse of 100us (t_WOL) */
    gpio_clear(i2c_pin_sda(cfg->atcai2c.bus));
    atca_delay_us(100);

    /* reinit I2C peripheral */
    i2c_init_pins(cfg->atcai2c.bus);
#else
    /* send wake pulse by sending byte 0x00 */
    /* this requires the I2C clock to be 100kHz at a max */
    i2c_acquire(cfg->atcai2c.bus);
    i2c_write_byte(cfg->atcai2c.bus, ATCA_WAKE_ADDR, data[0], 0);
    i2c_release(cfg->atcai2c.bus);
#endif

    atca_delay_us(cfg->wake_delay);

    uint8_t retries = cfg->rx_retries;
    int status = -1;

    i2c_acquire(cfg->atcai2c.bus);
    while (retries-- > 0 && status != 0) {
        status = i2c_read_bytes(cfg->atcai2c.bus,
                                (cfg->atcai2c.slave_address >> 1),
                                &data[0], 4, 0);
    }
    i2c_release(cfg->atcai2c.bus);

    if (status != ATCA_SUCCESS) {
        printf("Communication with device failed\n");
        return;
    }

    if (hal_check_wake(data, 4)) {
        printf("Wake up failed\n");
    }
}

void atecc_idle(void)
{
    ATCAIfaceCfg *cfg = (ATCAIfaceCfg *)&atca_params[I2C_DEV(0)];
    i2c_acquire(cfg->atcai2c.bus);
    i2c_write_byte(cfg->atcai2c.bus, (cfg->atcai2c.slave_address >> 1), ATCA_IDLE_ADDR, 0);
    i2c_release(cfg->atcai2c.bus);
}

void atecc_sleep(void)
{
    ATCAIfaceCfg *cfg = (ATCAIfaceCfg *)&atca_params[I2C_DEV(0)];
    i2c_acquire(cfg->atcai2c.bus);
    i2c_write_byte(cfg->atcai2c.bus, (cfg->atcai2c.slave_address >> 1), ATCA_SLEEP_ADDR, 0);
    i2c_release(cfg->atcai2c.bus);
}

/**
 * Function to call RIOT software implementation of SHA-256
 */
static int test_riot_sha256(uint8_t *teststring, uint16_t len,
                            uint8_t *expected,
                            uint8_t *result)
{
    sha256_context_t ctx;

    sha256_init(&ctx);
    sha256_update(&ctx, (void *)teststring, len);
    sha256_final(&ctx, result);
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

int main(void)
{
    uint8_t teststring[] = "chili cheese fries";
    uint8_t expected[] =
    { 0x36, 0x46, 0xEF, 0xD6, 0x27, 0x6C, 0x0D, 0xCB, 0x4B, 0x07, 0x73, 0x41,
      0x88, 0xF4, 0x17, 0xB4, 0x38, 0xAA, 0xCF, 0xC6, 0xAE, 0xEF, 0xFA, 0xBE,
      0xF3, 0xA8, 0x5D, 0x67, 0x42, 0x0D, 0xFE, 0xE5 };

    uint8_t result[SHA256_HASH_SIZE];                       /* +3 to fit 1 byte length and 2 bytes checksum */

    memset(result, 0, SHA256_HASH_SIZE);                    /* alles in result auf 0 setzen */

    uint16_t test_string_size = (sizeof(teststring) - 1);   /* -1 to ignore \0 */

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

    return 0;
}
