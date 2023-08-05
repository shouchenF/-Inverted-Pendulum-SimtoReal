from stable_baselines3 import PPO
import torch
import sys
import re  # 提取文本中的特定类型字符
import matplotlib.pyplot as plt

sys.path.append("../../..")

import serial  # 导入串口通信模块
import time
from until.highPrecTimer import Timer as HpTimer
import csv
global result, action_
result = [0.0, 0.0, 0.0, 0.0]
action_ = 0
f = open('../plot_data/action.csv', 'w', encoding='utf-8', newline='')
csv_writer = csv.writer(f)
def task():
    ax = []  # 定义一个 x 轴的空列表用来接收动态的数据
    ypt = []  # 定义一个 y 轴的空列表用来接收动态的数据
    ypc = []  # 定义一个 y 轴的空列表用来接收动态的数据
    yvc = []  # 定义一个 y 轴的空列表用来接收动态的数据
    yac = []  # 定义一个 y 轴的空列表用来接收动态的数据
    ywc = []  # 定义一个 y 轴的空列表用来接收动态的数据
    plt.ion()  # 开启一个画图的窗口
    delay_us = 50 * 1000
    tic = HpTimer(delay_us)
    for i in range(100000):  # 遍历0-99的值
        tic.waiting()
        # time.sleep(0.050)
        ax.append(i) # 添加 i 到 x 轴的数据中
        ypt.append(action_)  # 添加 i 的平方到 y 轴的数据中
        ypc.append(result[0])  # 添加 i 的平方到 y 轴的数据中
        yac.append(result[1])  # 添加 i 的平方到 y 轴的数据中
        yvc.append(result[2])  # 添加 i 的平方到 y 轴的数据中
        ywc.append(result[3])  # 添加 i 的平方到 y 轴的数据中
        plt.clf()  # 清除之前画的图
        plt.plot(ax, ypt, 'k')  # 画出当前 ax 列表和 ay 列表中的值的图形
        plt.plot(ax, ypc, 'r')  # 画出当前 ax 列表和 ay 列表中的值的图形
        plt.plot(ax, yac, 'b')  # 画出当前 ax 列表和 ay 列表中的值的图形
        plt.plot(ax, yvc, 'm')  # 画出当前 ax 列表和 ay 列表中的值的图形
        plt.plot(ax, ywc, 'g')  # 画出当前 ax 列表和 ay 列表中的值的图形
        plt.pause(0.001)  # 暂停一秒
        plt.ioff()  # 关闭画图的窗口
        plt.savefig('plot.svg')  # 保存当前绘图为图片文件

def count_odd_numbers(action):
    count = 0
    action_str = str(action)
    for c in action_str:
        if c.isdigit() and int(c) % 2 == 1:
            count += 1
    return count


def run_play():
    global result, action_
    one_flag = 1
    last_result = [0.0, 0.0, 0.0, 0.0]

    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    policy_net = PPO.load("../network_data/ppo_pen_51200.zip")

    ser = serial.Serial(  # 下面这些参数根据情况修改
        port='COM8',  # 串口
        baudrate=921600,  # 波特率
        timeout=0.001
        # parity=serial.PARITY_ODD,
        # stopbits=serial.STOPBITS_TWO,
        # bytesize=serial.SEVENBITS
    )
    if ser.isOpen():  # 判断串口是否打开
        print("open success")

    every_time = time.strftime('%Y-%m-%d %H:%M:%S')  # 时间戳
    data = ''
    delay_us = 5 * 1000
    tic = HpTimer(delay_us)
    for i in range(1000000):
        tic.waiting()
        ########## 1、 接收倒立摆的状态信息 ##############
        if ser.in_waiting >= 39:
            print(ser.in_waiting)
            data = ser.read(ser.in_waiting)
            # data = ser.readline()
            print("data", data)
            data_str = data.decode('utf-8')
            pattern = r"(-?\d+\.\d+)\s*,\s*(-?\d+\.\d+)\s*,\s*(-?\d+\.\d+)\s*,\s*(-?\d+\.\d+)\s*"
            matches = re.findall(pattern, data_str)
            # 将匹配结果转换为浮点数并存放到数组中
            if matches:
                result = [float(value) for value in matches[0]]
            print("task1——result=", result)

        start_time = time.time()
        obs = torch.tensor(result, dtype=torch.float32).to(device)
        action = policy_net.predict(obs)
        end_time = time.time()
        # print(end_time, end_time - start_time)
        # ######### 3、 发送神经网络输出的动作信息 ###########
        action_ = -0.20 + (0.5 * (action[0][0] + 1.0) * (0.20 - (-0.20)))
        # action_ = min(max(action[0][0], -0.18), 0.18)
        # action = tuple(max(min(a, 0.40), 0.05) if a is not None else None for a in action[0])
        # 将action转化成字符串
        action_str = '\t'  # 帧头
        action_str += '{}'.format(count_odd_numbers(action_))  # 奇偶校验位
        # action_str += ''.join(str(a) for a in action[0])  # 数据位
        action_str += str(action_)
        action_str += "\r\n"  # 帧尾
        # print(action_str.encode("utf-8"))
        print(action_, result[0], result[1], result[2], result[3])
        csv_writer.writerow([action_, result[0], result[1], result[2], result[3]])
        ser.write(action_str.encode("utf-8"))  # 向端口些数据 字符串必须译码

    ser.close()  # 关闭串口


if __name__ == '__main__':
    # t = Thread(target=task, args=())
    # t.start()
    run_play()