import struct

import sys

sys.path.append("../../..")

import serial  # 导入串口通信模块
from until.highPrecTimer import Timer as HpTimer
import csv
global result, action_

f = open('../plot_data/action.csv', 'w', encoding='utf-8', newline='')
csv_writer = csv.writer(f)

Bias = 0.0
D_Bias = 0.0
Last_Bias = 0.0
Position_Least = 0.0
Position_Bias = 0.0
Position_Differential = 0.0
Last_Position = 0.0
Position_PWM = 0.0
ZHONGZHI = 3100
Balance_KP = 400
Balance_KD = 400
Position_KP = 20
Position_KD = 300
Swing_up = 0

def count_odd_numbers(action):
    count = 0
    action_str = str(action)
    for c in action_str:
        if c.isdigit() and int(c) % 2 == 1:
            count += 1
    return count

def Balance(Angle):
    global Bias, D_Bias, Last_Bias
    Bias = Angle - ZHONGZHI
    D_Bias = Bias - Last_Bias
    balance = -Balance_KP * Bias - D_Bias * Balance_KD
    Last_Bias = Bias
    return balance


def Position(Encoder):
    global Position_Least, Position_Bias, Position_Differential, Last_Position, Position_PWM
    Position_Least = Encoder - Position_Zero
    Position_Bias *= 0.8
    Position_Bias += Position_Least * 0.2
    Position_Differential = Position_Bias - Last_Position
    Last_Position = Position_Bias
    Position_PWM = Position_Bias * Position_KP + Position_Differential * Position_KD
    return Position_PWM

def Xianfu_Pwm(Moto):
    Amplitude = 6900
    if Moto < -Amplitude:
        Moto = -Amplitude
    if Moto > Amplitude:
        Moto = Amplitude
    return Moto

def control_motor(Encoder, Angle_Balance):  #  //Angle_Balance:角位移传感器数据
    global Swing_up, Position_Zero, Last_Position, Last_Bias, Position_Target, Flag_Stop, Position_Pwm
    if Swing_up == 0: # 用于标记手动起摆时，是否是第一次进入手动起摆函数
        Position_Zero = Encoder
        Last_Position = 0
        Last_Bias = 0
        Position_Target = 0
        Swing_up = 1

    Balance_Pwm = Balance(Angle_Balance) # 角度PD控制
    # if Position_Target > 4:
    Position_Pwm = Position(Encoder)
    Position_Target = 0 # 位置PD控制 25ms进行一次位置控制
    Moto = Balance_Pwm - Position_Pwm # 计算电机最终PWM
    Moto = Xianfu_Pwm(Moto) # PWM限幅 防止占空比100%带来的系统不稳定因素
    return Moto

def Byte2Float(byteArry):
    floatNum = struct.unpack('f', byteArry)[0]
    return floatNum

# 从串口接收的数据为：编码器（整型）、角位移传感器（浮点型）
def read_serial_one_data_encoder_adc(ser):
    global receive_result
    BUF_SIZE = 8
    buf = bytearray([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
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
            if ib == 8:
                if buf[6] == 0x56 and buf[7] == 0x78:
                    Encoder = (buf[3] << 8) + buf[2]
                    Adc= (buf[5] << 8) + buf[4]
                    receive_result = [Encoder, Adc]
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

# 从串口接收的数据为：电机位置（整型）、电机速度（浮点型）
def read_serial_one_data_motor_position_velocity(ser):
    global receive_result
    BUF_SIZE = 10
    buf = bytearray([0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56, 0x78])
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
            if ib == 10:
                if buf[8] == 0x56 and buf[9] == 0x78:
                    motor_position = (buf[3] << 8) + buf[2]
                    # motor_veclocity = (buf[7] << 24) +(buf[6] << 16) +(buf[5] << 8) + buf[4]
                    motor_veclocity = Byte2Float(buf[4:8])
                    receive_result = [motor_position, motor_veclocity]
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


def run_play():
    result = [0, 0]
    action = bytearray([0x12, 0x34, 0x00, 0x00, 0x00, 0x56, 0x78])
    ser = serial.Serial(  # 下面这些参数根据情况修改
        port='COM8',  # 串口
        baudrate=921600,  # 波特率
        timeout=None,
        parity=serial.PARITY_ODD, #
        stopbits=serial.STOPBITS_ONE,
        bytesize=8
    )
    if ser.isOpen():  # 判断串口是否打开
        print("open success")
    delay_us = 5 * 1000
    tic = HpTimer(delay_us)
    for i in range(1000000):
        tic.waiting()
        ########## 1、 接收倒立摆的状态信息 ##############
        # c = read_serial_one_data_motor_position_velocity(ser)
        result = read_serial_one_data_encoder_adc(ser)
        # R = ser.read(1)
        # print("data", R)
        print("data", result)
        #
        # # # # ######### 2、 将状态信息输入电机控制代码中 ###########
        action_ = int(control_motor(result[0], result[1]))
        print("action_", action_)
        # # 将action转化成字符串
        action[0] = 0x2D   # 帧头
        action[1] = 0x01
        # action[2] = 0x45  # 帧头
        # action[3] = 0x31
        # action[4] = 0x32  # 帧头
        if action_ < 0:
            action[2] = 0x45   # 符号位
        action[3] = action_  & 0xFF    # 数据位
        action[4] = (action_ >>8) & 0xFF  # 数据位
        action[5] = 0x56   # 0x56是十六进制表示法，表示的是十进制数值86。而V是英文字母，它在ASCII码中的十进制表示是86。所以，0x56和V表示的是同一个字符。
        action[6] = 0x78   # 0x78是十六进制表示法，表示的是十进制数值120。而x是英文字母，它在ASCII码中的十进制表示是120。所以，0x78和x表示的是同一个字符。
        ser.write(action) # 使用DMA需要多个字节一起发送
        #
        # for byte in action: # 一个一个字节发送 ，一次发送多个字节容易出错
            # ser.write(byte.to_bytes(1, 'big'))

if __name__ == '__main__':
    run_play()