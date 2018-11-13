#!/usr/bin/env python3

# Copyright (C) 2018 Kevin Weiss <kevin.weiss@haw-hamburg.de>
# Copyright (C) 2018 Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import puf_sram_if
import numpy
import time


def min_erntropy(all_meas):
    p1 = numpy.zeros(len(all_meas[0]))
    # number of ones for each bit
    for i in range(0, len(all_meas[0])):
        tmp = list(map(lambda x: int(x[i]), all_meas))
        p1[i] = numpy.count_nonzero(tmp)

    # probability of ones
    p1 = numpy.divide(p1, float(len(all_meas)))
    # probability of zeros
    p0 = 1 - p1

    p0_1_max = numpy.maximum(p1, p0)
    log2_p0_1_max = numpy.log2(p0_1_max)
    H_min = numpy.sum(-log2_p0_1_max)
    H_min_rel = 100 * H_min/len(p0_1_max)

    return [H_min, H_min_rel]


def main_func():
    puf_sram = puf_sram_if.PufSram()
    time_start = time.time()
    data = puf_sram.get_seed_list(n=500, off_time=0, allow_print=True, write_logfiles=True)
    time_end = time.time()

    # evaluate random seeds
    seeds32 = [format(x, '0>32b') for x in data[0]]
    H_min_32, H_min_rel_32 = min_erntropy(seeds32)

    # evaluate IDs
    seeds160 = [format(x, '0>160b') for x in data[1]]
    H_min_160, H_min_rel_160 = min_erntropy(seeds160)

    print("Time for experiment   : %i seconds" % (time_end-time_start))
    print("Number of measurements: %i       " % len(seeds32))
    print("Seed 1 length         : %i Bit   " % len(seeds32[0]))
    print("Seed 1 abs. Entropy   : %02.02f Bit   " % H_min_32)
    print("Seed 1 rel. Entropy   : %02.02f perc. " % H_min_rel_32)
    print("Seed 2 length         : %i Bit   " % len(seeds160[0]))
    print("Seed 2 abs. Entropy   : %02.02f Bit   " % H_min_160)
    print("Seed 2 rel. Entropy   : %02.02f perc. " % H_min_rel_160)


if __name__ == "__main__":
    main_func()
