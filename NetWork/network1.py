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




def CRC16(data, length):
    CRC = 0xFFFF
    for i in range(length):
        CRC ^= data[i]
        for _ in range(8):
            if CRC & 0x0001:
                CRC = (CRC >> 1) ^ 0xA001
            else:
                CRC >>= 1
    return CRC


def Byte2Float(byteArry):
    floatNum = struct.unpack('f', byteArry)[0]
    return floatNum


def FloatToByte(floatNum):
    byteArry = bytearray(struct.pack('f', floatNum))
    return byteArry


# 从串口接收的数据为：电机位置（浮点型）、电机速度（浮点型）
def read_serial_one_data_motor_position_velocity(ser):
    receive_result = [1, 1]
    BUF_SIZE = 12
    buf = bytearray([0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56, 0x78])
    c1 = c2 = ib = flag = 0

    while True:
        R = ser.read(1)
        # print("data", R)
        if R == b'':
            print("Read Fail")
            ser.close()
            break
        c = int.from_bytes(R, byteorder='big')
        # print("data", c)
        if flag > 0:
            if ib < BUF_SIZE:
                buf[ib] = c
                ib += 1
            if ib == 12:
                if buf[10] == 0x56 and buf[11] == 0x78:
                    motor_position = Byte2Float(buf[2:6])  # 待修改
                    # motor_veclocity = (buf[7] << 24) +(buf[6] << 16) +(buf[5] << 8) + buf[4]
                    motor_veclocity = Byte2Float(buf[6:10])
                    receive_result = [round(motor_position, 4), round(motor_veclocity, 4)]
                    # receive_result = [motor_position, motor_veclocity]
                    break
                else:
                    print("CRC Fail")
                flag = 0
        if flag == 0:
            if c1 == 0x12 and c == 0x34:
                flag = 1
                ib = 2
        c1 = c
    return receive_result

# 从串口接收的数据为：电机位置（浮点型）、电机速度（浮点型）
def read_serial_all_data_position_velocity(ser):
    receive_result = [1.0, 1.0, 1.0, 1.0]
    BUF_SIZE = 20
    buf = bytearray([0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56, 0x78])
    c1 = ib = flag = 0

    while True:
        R = ser.read(1)
        # print("data", R)
        if R == b'':
            print("Read Fail")
            ser.close()
            break

        c = int.from_bytes(R, byteorder='big')
        # print("data", c)
        if flag > 0:
            if ib < BUF_SIZE:
                buf[ib] = c
                ib += 1
            if ib == 20:
                # print(buf)
                if buf[18] == 0x56 and buf[19] == 0x78:
                    motor_position = Byte2Float(buf[2:6])
                    motor_veclocity = Byte2Float(buf[6:10])
                    sensor_position = Byte2Float(buf[10:14])
                    sensor_veclocity = Byte2Float(buf[14:18])
                    receive_result = [round(motor_position, 4), round(motor_veclocity, 4), round(sensor_position, 4), round(sensor_veclocity, 4)]
                    # receive_result = [motor_position, motor_veclocity]
                    break
                else:
                    print("CRC Fail")
                flag = 0
        if flag == 0:
            if c1 == 0x12 and c == 0x34:
                flag = 1
                ib = 2
        c1 = c
    return receive_result

def read_serial_two_data_encoder_position_velocity(ser, abs):
    c1 = c2 = ib = flag = 0
    result_encoder = [0.0, 0.0]
    while True:
        R = ser.read(1)
        if R == b'':
            print("Read Fail")
            ser.close()
            break
        c = int.from_bytes(R, byteorder='big')

        if flag > 0:
            if ib < abs.BUF_SIZE:
                abs.buf[ib] = c
                ib += 1
            if ib == abs.BUF_SIZE:
                CRC = abs.CRC16(abs.buf, abs.BUF_SIZE - 3)
                if (CRC & 0xFF) == abs.buf[7] and ((CRC >> 8) & 0xFF) == abs.buf[8]:
                    a_c = (abs.buf[5] << 8) + abs.buf[6]
                    abs.angle(a_c)
                    abs.angle_total()
                    abs.velocity(abs.angle_total_c, abs.angle_total_pre)
                    abs.angle_total_pre = abs.angle_total_c
                    abs.angle_pre = abs.angle_c
                    result_encoder = [round(abs.angle_total_c, 4), round(abs.velocity_c, 4)]
                    # print(f"a_c {abs.angle_total_c:10.2f} , a_pre {abs.angle_total_pre:5.2f} , v_c {abs.velocity_c:10.5f} , counter {abs.counter:5d}")
                    break
                else:
                    print("CRC Fail")
                flag = 0
        if flag == 0:
            if c2 == 0x01 and c1 == 0x03 and c == 0x04:
                flag = 1
                ib = 3
        c2 = c1
        c1 = c
    return result_encoder


def run_play():
    result1 = [0.0, 0.0]
    result2 = [0.0, 0.0]
    result = [0.0, 0.0, 0.0, 0.0]
    absence = Sensor.ABSENC()
    action_send = bytearray([0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x56, 0x78])
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    policy_net = PPO.load("./ppo_pen_51200.zip")
    ser1 = serial.Serial(  # 下面这些参数根据情况修改
        port='COM8',  # 串口
        baudrate=921600,  # 波特率
        timeout=None,
        parity=serial.PARITY_ODD,  #
        stopbits=serial.STOPBITS_ONE,
        bytesize=8
    )

    # ser2 = serial.Serial(port="COM9",
    #                      baudrate=115200,
    #                      bytesize=8,
    #                      parity=serial.PARITY_NONE,
    #                      stopbits=1,
    #                      timeout=None)

    if ser1.isOpen():  # 判断串口是否打开
        print("ser1 open success")
    else:
        print("ser1 Open Fail")

    # if ser2.isOpen():  # 判断串口是否打开
    #     print("ser2 open success")
    # else:
    #     print("ser2 Open Fail")

    while True:
        ########## 1、 接收倒立摆的状态信息 ##############
        result1 = read_serial_all_data_position_velocity(ser1)
        print(result1)
        # result2 = read_serial_two_data_encoder_position_velocity(ser2, absence)
        # print(result2)
        # result = [result1[0], result2[0], result1[0], result2[1]]
        result = [result1[0], result1[1], result1[2], result1[3]]
        obs = torch.tensor(result, dtype=torch.float32).to(device)
        action = policy_net.predict(obs)
        action_ = -0.70 + (0.5 * (action[0][0] + 1.0) * (0.70 - (-0.70)))
        action_ = -2.87
        bytes_array = FloatToByte(action_)  # 十进制转换成单精度浮点数
        # # # 将action转化成字符串
        action_send[0] = 0x2D  # 帧头
        action_send[1] = 0x01
        action_send[2] = bytes_array[0]   # 数据位
        action_send[3] = bytes_array[1]   # 数据位
        action_send[4] = bytes_array[2]   # 数据位
        action_send[5] = bytes_array[3]   # 数据位
        action_send[6] = 0x56  # 0x56是十六进制表示法，表示的是十进制数值86。而V是英文字母，它在ASCII码中的十进制表示是86。所以，0x56和V表示的是同一个字符。
        action_send[7] = 0x78  # 0x78是十六进制表示法，表示的是十进制数值120。而x是英文字母，它在ASCII码中的十进制表示是120。所以，0x78和x表示的是同一个字符。
        print(action_send)
        ser1.write(action_send)  # 使用DMA需要多个字节一起发送
        # ######### 3、 发送神经网络输出的动作信息 ###########
        # send_data(ser2, action[0][0])


if __name__ == '__main__':
    run_play()
