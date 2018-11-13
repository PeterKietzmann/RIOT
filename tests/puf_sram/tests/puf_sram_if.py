#!/usr/bin/env python3

# Copyright (C) 2018 Kevin Weiss <kevin.weiss@haw-hamburg.de>
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import serial
import time


class PufSram:

    def __init__(self, port='/dev/ttyUSB0', baud=115200):
        self.__dev = serial.Serial(port, baud, timeout=10)
        if(self.__dev.isOpen() is False):
            self.__dev.open()

    def repower(self, shutdown_time=1):
        self.__dev.setRTS(True)
        time.sleep(shutdown_time)
        self.__dev.setRTS(False)

    def read_data(self):
        data = None
        seed1 = None
        seed2 = None
        start = False
        str = 'no_exit'
        while (str != ''):
            str = self.__dev.readline()
            if (b'Start: ' in str):
                start = True
            if ((b'Success: ' in str) and (start is True)):
                if (b'[' in str) and (b']' in str):
                    data_str = str[str.find(b"[")+1:str.find(b"]")]
                    data = data_str.split()
                    seed1 = int(data_str.split()[0], 0)
                    seed2 = int(data_str.split()[1], 0)
            if ((b'End: ' in str) and (data is not None)):
                return [seed1, seed2]
        return None

    def get_seed_list(self, n=10000, off_time=1, allow_print=False, write_logfiles=True):
        seeds1 = list()
        seeds2 = list()
        for i in range(0, n):
            self.repower(off_time)
            data = self.read_data()
            seeds1.append(data[0])
            seeds2.append(data[1])
            if (allow_print):
                print('Iteration %i/%i' % (i, n))
                print('seed1 %i' % seeds1[-1])
                print('seed2 %i' % seeds2[-1])
            if (write_logfiles):
                file_seed1 = open("seed1_32b.dat", 'a')
                file_seed2 = open("seed2_160b.dat", 'a')
                file_seed1.write(str(format(seeds1[-1], '0>32b'))+'\n')
                file_seed2.write(str(format(seeds2[-1], '0>160b'))+'\n')
                file_seed1.close()
                file_seed2.close()
        return [seeds1, seeds2]

    def connect(self, dev):
        if (dev.isOpen()):
            dev.close()
        self.__dev = self
        if(self.__dev.isOpen() is False):
            self.__dev.open()

    def disconnect(self):
        self.__dev.close()

    def __del__(self):
        self.__dev.close()
