from stable_baselines3 import PPO
import hydra
import torch
import sys
import re  # 提取文本中的特定类型字符
sys.path.append("../..")

import serial  # 导入串口通信模块
import time

result = [0.0, 0.0, 0.0, 0.0]
offset = [0.0]
joint_vel = [0.0]
ctrl = [0.0]
target = [0.0]

def sendData():
    i = 0
    sum = 0x00
    sendBuffer = [
            170, 85, 85, 85,
            85, 85, 85, 85,
            85, 47,]
    for i in range(1, 7):
        sum += sendBuffer[i]
    sendBuffer[8] = sum
    # print(sendBuffer)
    separator = '-'
    sendBuffer_str = separator.join(map(str, sendBuffer))
    # sendBuffer_str = ''.join(str(i) for i in sendBuffer)
    # print("发送的数据", sendBuffer_str.encode("utf8"))
    ret = ser.write(sendBuffer_str.encode("utf8"))

    return ret

class MyNetwork(torch.nn.Module):
    def __init__(self, input_dim: int, output_dim: int, hidden_dim: int, model_path: str):
        super(MyNetwork, self).__init__()
        self.fc1 = torch.nn.Linear(input_dim, hidden_dim)
        self.fc2 = torch.nn.Linear(hidden_dim, hidden_dim)
        self.fc3 = torch.nn.Linear(hidden_dim, output_dim)
        model_a = PPO.load(model_path)
        # model_a = PPO.load("../../log/M04/2023-05-29/14-23-46/model_saved/ppo_M04_2150400.zip")
        params = model_a.policy.state_dict()
        # for name, value in params.items():
        #     print(name, value)
        self.fc1.weight.data = params["mlp_extractor.policy_net.0.weight"]
        self.fc1.bias.data = params["mlp_extractor.policy_net.0.bias"]
        self.fc2.weight.data = params["mlp_extractor.policy_net.2.weight"]
        self.fc2.bias.data = params["mlp_extractor.policy_net.2.bias"]
        self.fc3.weight.data = params["action_net.weight"]
        self.fc3.bias.data = params["action_net.bias"]

    def forward(self, obs_a: torch.Tensor) -> torch.Tensor:
        x = torch.relu(self.fc1(obs_a))
        x = torch.relu(self.fc2(x))
        return torch.tanh(self.fc3(x))

def step_simulation(action):
    target = action + offset
    # 关节速度 = 关节初始速度 + 传感器获取速度
    joint_vel[0] = 0.7 * joint_vel[0] + 0.3 * result[1]
    ctrl[0] = 1500 * (target[0] - result[0]) - 100 * joint_vel[0]
    return ctrl


def run_play():
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    policy_netb = MyNetwork(4, 1, 128,"./ppo_M04_240640.zip")
    policy_netb.to(device)
    policy_net = policy_netb

    every_time = time.strftime('%Y-%m-%d %H:%M:%S')  # 时间戳
    data = ''
    last_result = [0.0, 0.0, 0.0, 0.0]
    for i in range(10000):
        ########## 1、 接收倒立摆的状态信息 ##############
        data = ser.readline()
        # 将字节串转换为字符串
        data_str = data.decode('utf-8')
        # 解决数据丢失问题
        # data = "motor_position=0.0000; motor_velocity=0.0000; sensor_position=95.3613; sensor_velocity=87.8906;"
        # 使用正则表达式匹配出目标字符串
        pattern = r"motor_position=(\d+\.\d+); motor_velocity=(\d+\.\d+); sensor_position=(\d+\.\d+); sensor_velocity=([-+]?\d+\.\d+);"
        # pattern = r"[-+]?\d*\.\d+|\d+"  # 匹配浮点数或整数
        print(data_str)

        # match = re.search(pattern, data)
        # if match:
        #     motor_position = float(match.group(1))
        #     motor_velocity = float(match.group(2))
        #     sensor_position = float(match.group(3))
        #     sensor_velocity = float(match.group(4))
        #     print("motor_position =", motor_position)
        #     print("motor_velocity =", motor_velocity)
        #     print("sensor_position =", sensor_position)
        #     print("sensor_velocity =", sensor_velocity)

        matches = re.findall(pattern, data_str)

        # 将匹配结果转换为浮点数并存放到数组中

        if matches:
            result = [float(value) for value in matches[0]]
            last_result = result
        else:
            result = last_result

        print(every_time, result)
       ############2、 神经网络动作处理    ###############
        # tensor_result = torch.tensor(result)
        # obs = torch.from_numpy(tensor_result).float().to(device)
        obs = torch.tensor(result, dtype=torch.float32).to(device)
        action = policy_net.forward(obs)
        action = action.detach().cpu().numpy()
        ######### 3、 发送神经网络输出的动作信息 ###########
        # 将action转化成字符串
        action = step_simulation(action)
        print(action)
        action_str = '\r\n'.join(str(a) for a in action)
        # action_str = str(action)
        # print(action_str.encode("utf8"))
        ser.write(action_str.encode("utf8"))  # 向端口些数据 字符串必须译码

if __name__ == '__main__':
    ser = serial.Serial(  # 下面这些参数根据情况修改
        port='COM3',  # 串口
        baudrate=115200,  # 波特率
        parity=serial.PARITY_ODD,
        stopbits=serial.STOPBITS_TWO,
        bytesize=serial.SEVENBITS
    )
    # 串口执行到这已经打开 再用open命令会报错
    if ser.isOpen():  # 判断串口是否打开
        print("open success")
        # ser.write("hello".encode("utf8"))  # 向端口些数据 字符串必须译码
    run_play()