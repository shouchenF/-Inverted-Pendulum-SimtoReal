# -*- coding:utf-8 -*-
# 用于提供较高精度的时间差,根据文献，精度可达微秒级

import time, ctypes
# 提供实例方法，获取时间差
#  __freq = 10000000 ==> 0.1us; 10000 cnt = 1ms#
class Timer(object):
    def __init__(self, delay_us):  # 1us
        freq = ctypes.c_longlong(0)
        ctypes.windll.kernel32.QueryPerformanceFrequency(ctypes.byref(freq))
        self.__freq = freq.value
        self.__freq_us = self.__freq/1000000    #counter/us
        self.__beginCount = self.counter
        self.__dconter = delay_us * self.__freq_us

    def waiting(self):
        while self.counter - self.__beginCount < self.__dconter:
            j = 1
            # print(0)
        self.__beginCount += self.__dconter

    @property
    def counter(self):
        freq = ctypes.c_longlong(0)
        ctypes.windll.kernel32.QueryPerformanceCounter(ctypes.byref(freq))
        return freq.value


