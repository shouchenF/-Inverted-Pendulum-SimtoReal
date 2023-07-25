# -*- coding:utf-8 -*-
# 测试用代码
import time
import ctypes
from highPrecTimer import Timer as HpTimer
delay_us = 10*1000
tic = HpTimer(delay_us)
for i in range(100):
    print(time.time())
    # print(time.time())
    time.sleep(0.002) # 9ms
    tic.waiting()

