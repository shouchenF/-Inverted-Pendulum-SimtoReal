from stable_baselines3 import PPO
import torch
import sys
import re  # 提取文本中的特定类型字符
import matplotlib.pyplot as plt
sys.path.append("../..")

import serial # 导入串口通信模块
from threading import Thread, Event
import time
global result
import queue



def task(result_queue,
		 # event
		 ):
	ax = []  # 定义一个 x 轴的空列表用来接收动态的数据
	ay = []  # 定义一个 y 轴的空列表用来接收动态的数据
	plt.ion()  # 开启一个画图的窗口
	for i in range(100000):  # 遍历0-99的值
		# event.wait()  # 等待event被设置为True
		result = result_queue.get()
		# print("result=",result)
		ax.append(result[0])  # 添加 i 到 x 轴的数据中
		ay.append(result[0] ** 2)  # 添加 i 的平方到 y 轴的数据中
		plt.clf()  # 清除之前画的图
		plt.plot(ax, ay)  # 画出当前 ax 列表和 ay 列表中的值的图形
		plt.pause(0.1)  # 暂停一秒
		plt.ioff()  # 关闭画图的窗口

def count_odd_numbers(action):
	count = 0
	action_str = str(action)
	for c in action_str:
		if c.isdigit() and int(c) % 2 == 1:
			count += 1
	return count

def task1(result_queue,
		  # event
		  ):
	one_flag = 1
	last_result = [0.0, 0.0, 0.0, 0.0]

	device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
	policy_net = PPO.load("./ppo_M04_240640.zip")

	every_time = time.strftime('%Y-%m-%d %H:%M:%S')  # 时间戳
	data = ''
	ser = serial.Serial(  # 下面这些参数根据情况修改
		port='COM5',  # 串口
		baudrate=115200,  # 波特率
		timeout=0.001
		# parity=serial.PARITY_ODD,
		# stopbits=serial.STOPBITS_TWO,
		# bytesize=serial.SEVENBITS
	)
	# 串口执行到这已经打开 再用open命令会报错
	if ser.isOpen():  # 判断串口是否打开
		print("open success")

	for i in range(10000):
		time.sleep(0.04)
		########## 1、 接收倒立摆的状态信息 ##############
		if ser.in_waiting >= 102:
			print(ser.in_waiting)
			data = ser.read(ser.in_waiting)
			print("data", data)
			# 将字节串转换为字符串
			data_str = data.decode('utf-8')
			# 解决数据丢失问题
			# 使用正则表达式匹配出目标字符串
			pattern = r"motor_position=(-?\d+\.\d+)\s*;\s*sensor_position=(-?\d+\.\d+)\s*;\s*motor_velocity=(-?\d+\.\d+)\s*;\s*sensor_velocity=(-?\d+\.\d+)\s*;"
			matches = re.findall(pattern, data_str)

			# 将匹配结果转换为浮点数并存放到数组中
			if matches:
				receive_result = [float(value) for value in matches[0]]
				last_result = receive_result
			else:
				receive_result = last_result

			result = receive_result
			print("task1——result=", result)
			result_queue.put(result)
			# event.set()  # 设置event为True，通知task线程可以获取result的值

			if one_flag == 1:
				result[0] = 0.0
				result[1] = 0.0
				result[2] = 0.0
				result[3] = 0.0
				one_flag += 1

			############2、 神经网络动作处理    ###############
			obs = torch.tensor(result, dtype=torch.float32).to(device)
			action = policy_net.predict(obs)

			######### 3、 发送神经网络输出的动作信息 ###########
			action = tuple(max(min(a, 0.4), 0.05) if a is not None else None for a in action[0])
			# 将action转化成字符串
			# print(action)

			action_str = '\t'  # 帧头
			action_str += '{}'.format(count_odd_numbers(action[0]))  # 奇偶校验位
			# action_str += ''.join(str(a) for a in action[0])  # 数据位
			action_str += str(action[0])
			action_str += "\r\n"  # 帧尾
			print(action_str.encode("utf-8"))
			ser.write(action_str.encode("utf-8"))  # 向端口些数据 字符串必须译码
	ser.close()  # 关闭串口

if __name__ == '__main__':
	result = [0.0, 0.0, 0.0, 0.0]
	result_queue = queue.Queue()
	# event = Event()  # 创建一个线程同步的Event对象
	# 因为创建线程不需要重新开辟内存空间，所以不用写main，创建线程只是单独把启动线程函数里面的代码拿出来用
	t1 = Thread(target=task1, args=(result_queue,
									# event
									))
	t1.start()
	t = Thread(target=task, args=(result_queue,
								  # event
								  ))
	t.start()
	print('主线程结束')