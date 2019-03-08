/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup   adc_util ADC I/O Utils
 * @ingroup    drivers_periph_adc
 * @brief      ADC utility functions
 *
 * @{
 * @file
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 */

#ifndef ADC_UTIL_H
#define ADC_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stddef.h>

#include "periph/adc.h"

/**
 * @brief   ADC configuration for random seed generation
 * @{
 */
#ifndef ADC_UTIL_NOISE_LINE
#define ADC_UTIL_NOISE_LINE         ADC_LINE(0)
#endif
#ifndef ADC_UTIL_NOISE_RES
#define ADC_UTIL_NOISE_RES          ADC_RES_10BIT
#endif
/** @} */

/**
 * @brief   Generates bytes from ADC noise
 *
 * @param[in] buf   pointer to write noisy bytes to
 * @param[in] len   number of bytes to generate
 *
 * @return          0 on succes
 * @return          -1 on ADC initialization failure
 */
int adc_util_sample_noise(uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* ADC_UTIL_H */
