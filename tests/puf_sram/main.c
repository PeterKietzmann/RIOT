/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Test application for the random number generator based of SRAM
 *
 * @author      Kevin Weiss <kevin.weiss@haw-hamburg.de>
 *
 * @}
 */
#include <inttypes.h>
#include <stdio.h>
#include "puf_sram.h"
#include "periph/pm.h"
#include "periph/rtc.h"

#define REBOOT_TIME_SEC     (1)


int main(void)
{
    puts("Start: Test random number generator");

    printf("Success: [0x%08" PRIX32 " ", puf_sram_seed);
    printf("0x");
    for(int i=0; i<20; i++){
        printf("%02x", puf_sram_seed_160[i]);
    }
    puts("]");

    puts("End: Test finished");

    struct tm time;
    rtc_get_time(&time);
    time.tm_sec += REBOOT_TIME_SEC;
    mktime(&time);
    rtc_set_alarm(&time, NULL, NULL);
    pm_set(0);

    return 0;
}
