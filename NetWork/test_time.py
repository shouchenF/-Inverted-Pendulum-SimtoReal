# -*- coding:utf-8 -*-
# 测试用代码
import time
import ctypes
from highPrecTimer import Timer as HpTimer
tic = ctypes.c_longlong(0) # 0.1us
tic_delay = 100000
t = HpTimer()
tic = t.counter()

for i in range(100):
    print(time.time())
    while t.counter() - tic < tic_delay:
        j = 1
    time.sleep(0.009)
    tic += tic_delay

