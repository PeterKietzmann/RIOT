/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_entropy
 *
 * @{
 * @file
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */
#include "entropy.h"

#ifdef MODULE_PERIPH_HWRNG
#include "periph/hwrng.h"
#endif
#ifdef MODULE_PERIPH_HWRNG
#include "periph/adc_util.h"
#endif

#define ENABLE_DEBUG        (0)
#include "debug.h"

/* Allocation of the entropy context */
static entropy_ctx_t ctx;

int entropy_init(void)
{
    DEBUG("%s\n", __FUNCTION__);

    // TODO change to better default?!
    // TODO how to integrate SRAM PUF here?!

#ifdef MODULE_PERIPH_HWRNG
    if(entropy_source_add((entropy_func_t)hwrng_read) < 0) {
        DEBUG("%s could not add HWRNG source\n", __FUNCTION__);
    }
#endif
#ifdef MODULE_PERIPH_ADC
    if(entropy_source_add((entropy_func_t)adc_util_sample_noise) < 0) {
        DEBUG("%s could not add ADC noise source\n", __FUNCTION__);
    }
#endif

    /* return error if no source was initialized */
    if(!ctx.idx) {
        return ENTROPY_NO_SRC;
    }
    /* return number of auto-initialized entropy sources */
    return ctx.idx;
}

int entropy_source_add(entropy_func_t func)
{
    DEBUG("%s\n", __FUNCTION__);

    int idx = ctx.idx;

    if(idx < ENTROPY_MAX_NUM_SOURCES) {
        ctx.func[idx] = func;
        ctx.idx++;
        return ENTROPY_OK;
    }
    return ENTROPY_MAX_SRC;
}

void entropy_reset(void)
{
    DEBUG("%s\n", __FUNCTION__);

    memset(&ctx, 0, sizeof(ctx));
}

int entropy_get(uint8_t *out, size_t len)
{
    DEBUG("%s\n", __FUNCTION__);

    /* return if too many bytes are requested */
    if(len > ENTROPY_MAX_BYTES_REQUEST) {
        DEBUG("%s: requested too many bytes\n", __FUNCTION__);
        return ENTROPY_ERR;
    }

    /* return if no entropy source is registered */
    if (!ctx.idx) {
        DEBUG("%s: no entropy source registered\n", __FUNCTION__);
        return ENTROPY_NO_SRC;
    }

    cipher_t cipher;
    uint8_t buf[ENTROPY_MAX_BYTES_REQUEST];
    uint8_t data[ENTROPY_MAX_BYTES_REQUEST];
    uint8_t key[ENTROPY_MAX_BYTES_REQUEST];

    /* initial data vector is zero */
    memset(data, 0, ENTROPY_MAX_BYTES_REQUEST);

    /* TODO: change to something smarter ? */
    for (int i = 0; i < ENTROPY_MAX_BYTES_REQUEST; i++) {
        key[i] = i;
    }

    /* returns 1 on success */
    if (cipher_init(&cipher, CIPHER_AES_128, key, ENTROPY_MAX_BYTES_REQUEST) < 1) {
        DEBUG("%s: failed to initialize cipher\n", __FUNCTION__);
        return ENTROPY_ERR;
    }

    for (int i = 0; i < ctx.idx; i++) {
        ctx.func[i](buf, ENTROPY_MAX_BYTES_REQUEST);
        for (int i = 0; i < ENTROPY_MAX_BYTES_REQUEST; i++) {
            buf[i] ^= data[i];
        }
        if(cipher_encrypt_ecb(&cipher, buf, ENTROPY_MAX_BYTES_REQUEST, data) < 0) {
            DEBUG("%s: failed to encrypt data\n", __FUNCTION__);
            crypto_secure_wipe(data, ENTROPY_MAX_BYTES_REQUEST);
            return ENTROPY_ERR;
        }
    }

    /* copy requested number of bytes to output */
    memcpy(out, data, len);

    /* set memory to zero. make sure this is not optimized out by the compilter */
    crypto_secure_wipe(data, ENTROPY_MAX_BYTES_REQUEST);

    return len;
}
