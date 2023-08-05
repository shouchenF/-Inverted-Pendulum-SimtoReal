import serial  # 导入串口通信模块
import time
import re  # 提取文本中的特定类型字符
def delay_ms(ms):
    time.sleep(ms / 1000)

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

ser = serial.Serial(  # 下面这些参数根据情况修改
  port='COM5',# 串口
  baudrate=115200,# 波特率
  parity=serial.PARITY_ODD,
  stopbits=serial.STOPBITS_TWO,
  bytesize=serial.SEVENBITS
)

# 串口执行到这已经打开 再用open命令会报错
if ser.isOpen():  # 判断串口是否打开
    print("open success")
    # ser.write("hello".encode("utf8"))  # 向端口些数据 字符串必须译码

every_time = time.strftime('%Y-%m-%d %H:%M:%S')# 时间戳
data = ''

while True:
    data = ser.readline()
    # 将字节串转换为字符串
    data_str = data.decode('utf-8')

    # 使用正则表达式匹配出目标字符串
    pattern = r"motor_position=(\d+\.\d+); motor_velocity=(\d+\.\d+); sensor_position=(\d+\.\d+); sensor_velocity=(\d+\.\d+);"
    matches = re.findall(pattern, data_str)

    # 将匹配结果转换为浮点数并存放到数组中
    result = []
    if matches:
        result = [float(value) for value in matches[0]]

    print(every_time, result)

    # 任务：实现发数据
    send = sendData()
    print("发送的数据", send)




