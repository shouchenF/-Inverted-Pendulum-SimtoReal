import pandas as pd
import matplotlib.pyplot as plt

# 读取CSV文件
data = pd.read_csv('action.csv')

# 提取第一列、第二列、第三列数据
col1 = data.iloc[:, 0].values
col2 = data.iloc[:, 1].values
col3 = data.iloc[:, 2].values
col4 = data.iloc[:, 3].values
col5 = data.iloc[:, 4].values

# 绘制折线图
# plt.plot(col1, label='action'        , c='blue')
# plt.plot(col2, label='motor_position', c='red')
# plt.plot(col3, label='sonsor_positon', c='green')
# plt.plot(col4, label='vc'            , c='black')
# plt.plot(col5, label='wc'            , c='yellow')

plt.plot(col1, 'k',label='action'        )  # 画出当前 ax 列表和 ay 列表中的值的图形
plt.plot(col2, 'r',label='motor_position')  # 画出当前 ax 列表和 ay 列表中的值的图形
plt.plot(col3, 'b',label='sonsor_positon')  # 画出当前 ax 列表和 ay 列表中的值的图形
plt.plot(col4, 'm',label='vc'            )  # 画出当前 ax 列表和 ay 列表中的值的图形
plt.plot(col5, 'g',label='wc'            )  # 画出当前 ax 列表和 ay 列表中的值的图形

# 设置图例、标题、坐标轴标签
plt.legend()
# plt.title('Data Visualization')
plt.xlabel('time')
plt.ylabel('(m)')

# 显示图形
plt.show()