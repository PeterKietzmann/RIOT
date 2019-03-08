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
#include "periph/adc_util.h"

#ifdef MODULE_PERIPH_ADC

int adc_util_sample_noise(uint8_t *out, size_t len)
{
    /* init ADC */
    if (adc_init(ADC_UTIL_NOISE_LINE) != 0) {
        return -1;
    }

    /* use LSB of each ADC sample and shift it to build a value */
    for (unsigned iter = 0; iter < len; iter++) {
        uint8_t byte = 0;
        for (unsigned bit = 0; bit < 8; bit++) {
            /* TODO check if samples are not equal, e.g.
               pulled to high or low voltage */
            int sample = adc_sample(ADC_UTIL_NOISE_LINE, ADC_UTIL_NOISE_RES);
            byte |= (uint8_t)(sample & 0x01) << bit;
        }
        out[iter] |= byte;
    }

    return 0;
}
#endif /* MODULE_PERIPH_ADC */
