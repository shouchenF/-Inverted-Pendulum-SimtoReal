import struct

from stable_baselines3 import PPO
import torch
import sys
import re  # 提取文本中的特定类型字符

sys.path.append("../..")
import serial.tools.list_ports
import serial  # 导入串口通信模块
import time

global result, action_
global modecmd1, modecmd2, modecmd3

import Sensor

def FloatToByte(floatNum):
    byteArry = bytearray(struct.pack('f', floatNum))
    return byteArry

if __name__ == '__main__':
    a = FloatToByte(0.12)
    print(a[2])