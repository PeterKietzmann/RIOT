/*
 * Copyright (C) 2018 HAW Hamburg
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
 * @brief       Test application for entropy gathering
 *
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "entropy.h"

int entropy_func_ext(uint8_t *buf, size_t len)
{
    printf("entropy_func_ext requested for %i bytes\n", len);

    /* this is just a stupid example that provides no entropy */
    for (size_t i = len; i > 0; i--) {
        buf[i] = i;
    }
    return len;
}

int main(void)
{
    uint8_t entropy[ENTROPY_MAX_BYTES_REQUEST];
    int ret;

    /* initialize module with default sources */
    ret = entropy_init();
    printf("Initialized %i entropy sources", ret);

    /* add external entropy function */
    ret = entropy_source_add(entropy_func_ext);
    if (ret != ENTROPY_OK) {
        puts("Failed to initialize entropy function");
    }
    else {
        puts("Succesfully added external entropy function");
    }

    /* get ENTROPY_MAX_BYTES_REQUEST bytes entropy data */
    ret = entropy_get(entropy, sizeof(entropy));
    if(ret < 0) {
        puts("Failed to get entropy value");
    }
    else {
        printf("Received %i bytes from entropy module\n", ret);
    }

    printf("Entropy value: ");
    for (size_t i = 0; i < sizeof(entropy); i++) {
        printf("%u", entropy[i]);
    }
    printf("\n");

    /* check that function returns error when requesting too many bytes */
    ret = entropy_get(entropy, sizeof(entropy)+1);
    if(ret == ENTROPY_ERR) {
        puts("Successfully received error return code");
    }
    else {
        puts("Failed to get error return code for invalid input size");
    }

    /* reset internal state of entropy module */
    entropy_reset();

    /* check that module reports error due to missing entropy sources after reset */
    ret = entropy_get(entropy, sizeof(entropy));
    if (ret == ENTROPY_NO_SRC) {
        puts("Successfully reset entropy module");
    }
    else {
        puts("Failed to reset entropy module");
    }
    return 0;
}
