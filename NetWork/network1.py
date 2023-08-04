from stable_baselines3 import PPO
import torch
import sys
import re  # 提取文本中的特定类型字符
sys.path.append("../..")
import serial.tools.list_ports
import serial  # 导入串口通信模块
import time
global result, action_

import Sensor

result1 = [0.0, 0.0]
result2 = [0.0, 0.0]

def count_odd_numbers(action):
    count = 0
    action_str = str(action)
    for c in action_str:
        if c.isdigit() and int(c) % 2 == 1:
            count += 1
    return count

def read_serial_one_data(ser, abs):
    c1 = c2 = ib = flag = 0

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
            else:
                CRC = abs.CRC16(abs.buf, abs.BUF_SIZE - 3)
                if (CRC & 0xFF) == abs.buf[7] and ((CRC >> 8) & 0xFF) == abs.buf[8]:
                    a_c = (abs.buf[5] << 8) + abs.buf[6]
                    abs.angle(a_c)
                    abs.angle_total()
                    abs.velocity(abs.angle_total_c, abs.angle_total_pre)
                    abs.angle_total_pre = abs.angle_total_c
                    abs.angle_pre = abs.angle_c
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
    return [0,0]

def read_serial_two_data(ser):
    c1 = ib = flag = 0
    receive_result = [0, 0]
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
            else:
                if abs.buf[7] == 0x56 and abs.buf[8] == 0x78:
                    motor_position = (abs.buf[2] << 8) + abs.buf[3]
                    motor_velocity = (abs.buf[4] << 8) + abs.buf[5]
                    receive_result = [motor_position, motor_velocity]
                    break
                else:
                    print("Fail")
                flag = 0
        if flag == 0:
            if  c1 == 0x34 and c == 0x12:
                flag = 1
                ib = 2
        c1 = c

    return receive_result

def send_data(ser, data):
    B = data & 0xFF  # 低8位
    A = (data >> 8) & 0xFF  # 高8位
    byte_array1 = bytearray([12, 34])
    byte_array2 = bytearray([B, A])
    byte_array3 = bytearray([56, 78])
    # 发送字节串到串口
    byte_array = byte_array1 + byte_array2 + byte_array3
    ser.write(byte_array)



def run_play():
    global result, action_, result1, result2
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    policy_net = PPO.load("./ppo_pen_51200.zip")
    ser1 = serial.Serial(port="COM15",
                         baudrate=115200,
                         bytesize=8,
                         parity=serial.PARITY_NONE,
                         stopbits=1,
                         timeout=None)

    ser2 = serial.Serial(port="COM15",
                         baudrate=115200,
                         bytesize=8,
                         parity=serial.PARITY_NONE,
                         stopbits=1,
                         timeout=0.001)

    if ser1.isOpen():  # 判断串口是否打开
        print("open success")
    else:
        print("Open Fail")

    while True:
        ########## 1、 接收倒立摆的状态信息 ##############
        if ser2.in_waiting >= 39:
            print(ser2.in_waiting)
            data = ser2.read(ser1.in_waiting)
            print("data", data)
            data_str = data.decode('utf-8')
            pattern = r"(-?\d+\.\d+)\s*,\s*(-?\d+\.\d+)\s*"
            matches = re.findall(pattern, data_str)
            # 将匹配结果转换为浮点数并存放到数组中
            if matches:
                result1 = [float(value) for value in matches[0]]
            print("task1——result=", result1)
            result2 = read_serial_one_data(ser1, abs)
        result = [result1[0], result2[0], result1[0], result2[1]]
        obs = torch.tensor(result, dtype=torch.float32).to(device)
        action = policy_net.predict(obs)
        # ######### 3、 发送神经网络输出的动作信息 ###########
        send_data(ser2, action[0][0])

if __name__ == '__main__':
    abs = Sensor.ABSENC()
    run_play()