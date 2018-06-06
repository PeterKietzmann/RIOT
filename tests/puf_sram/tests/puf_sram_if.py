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
        start = False
        str = 'no_exit'
        while (str != ''):
            str = self.__dev.readline()
            if ('Start: ' in str):
                start = True
            if (('Success: ' in str) and (start is True)):
                if ('[' in str) and (']' in str):
                    data_str = str[str.find("[")+1:str.find("]")]
                    data = int(data_str, 0)
            if (('End: ' in str) and (data is not None)):
                return data
        return None

    def get_seed_list(self, n=10000, off_time=1, allow_print=False):
        data = list()
        for i in range(0, n):
            self.repower(off_time)
            data.append(self.read_data())
            if (allow_print):
                print(data[-1])
        return data

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
