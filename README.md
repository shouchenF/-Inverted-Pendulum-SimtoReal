 基于STM32的一阶线性倒立摆强化学习控制策略从仿真到实物迁移 
· 在 Mujoco 仿真平台中，使用 URDF 和 XML 对系统进行建模，并使用 Pytorch 深度学习框架和 Stable-Baselines3 算法框架搭建单腿模型强化学习训练环境以及设计奖励函数，训练自动起摆的控制策略网络
· 使用 STM32 作为主控制器，通过 ADC 模块以及电机编码器实现倒立摆位置和速度的状态获取及电机控制
· 使用 Python 搭建上位机，用于将主控获取的状态输入到训练好的策略网络，推理出目标动作并下发给主控

 The first-order linear inverted pendulum reinforcement learning control strategy based on STM32 is transferred from simulation to physical object 
· In the Mujoco simulation platform, the system was modeled using URDF and XML, and the Pytorch deep learning framework and the Stable-Baselines3 algorithm framework were used to build a one-legged model reinforcement learning training environment and design reward functions to train an automatic swinging control strategy network
· Using the STM32 as the main controller, the state acquisition and motor control of the inverted pendulum position and speed are realized through the ADC module and the motor encoder
· Use Python to build a host computer to input the state obtained by the master into the trained policy network, deduce the target action and deliver it to the master controller
