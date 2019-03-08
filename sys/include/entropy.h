/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_entropy Entropy
 * @ingroup     sys
 *
 * @brief       Entropy extractor that accumulates entropy of multiple sources
 * @experimental
 *
 * @{
 * @file
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */
#ifndef ENROPY_H
#define ENROPY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#include "crypto/aes.h"
#include "crypto/helper.h"
#include "crypto/ciphers.h"
#include "crypto/modes/ecb.h"

/** Maximum number of entropy sources to accumulate */
#ifndef ENTROPY_MAX_NUM_SOURCES
#define ENTROPY_MAX_NUM_SOURCES    (3)
#endif

/** Maximum number of bytes that can be pruduces per function call */
#define ENTROPY_MAX_BYTES_REQUEST  (AES_KEY_SIZE)

/**
 * @brief   Named return values
 */
enum {
    ENTROPY_OK          =  0,    /**< everything was fine */
    ENTROPY_MAX_SRC     = -1,    /**< number of registered entropy sources exeeded */
    ENTROPY_NO_SRC      = -2,    /**< no registered entropy source available */
    ENTROPY_ERR         = -3     /**< other error */
};

/**
 * @brief   Signature of entropy poll functions
 *
 * @param[in] buf   pointer to write entropy data to
 * @param[in] len   number of bytes to generate
 *
 * @return          not yet used
 */
typedef int (*entropy_func_t)(uint8_t *buf, size_t len);

/**
 * @brief   Context for entropy accumulator
 */
typedef struct {
    int idx;                                     /**< number of registered entropy functions */
    entropy_func_t func[ENTROPY_MAX_NUM_SOURCES]; /**< entropy functions */
} entropy_ctx_t;

/**
 * @brief   Initialize entropy module
 *
 * @return          number of registered sources on success
 * @return          ENTROPY_NO_SRC if no entropy source was initialized
 */
int entropy_init(void);

/**
 * @brief   Add an entropy source to the accumulator.
 *
 * @param[in] cb    callback that is called during entropy gathering
 *
 * @return          ENTROPY_OK on success
 * @return          ENTROPY_MAX_SRC on error
 */
int entropy_source_add(entropy_func_t cb);

/**
 * @brief   Reset global state of entropy accumulator.
 */
void entropy_reset(void);

/**
 * @brief   Get accumulated entropy value
 *
 * @param[in] out   pointer to write entropy data to
 * @param[in] len   number of bytes to generate
 *
 * @return          number of bytes on success
 * @return          ENTROPY_NO_SRC if not source is available
 * @return          ENTROPY_ERR on other error
 */
int entropy_get(uint8_t *out, size_t len);

///**
// * @brief   Continous health test
// *
// * @return          not yet implemented
// */
// int entropy_continuous_test(void);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* ENROPY_H */
