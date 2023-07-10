/***********************************************
公司：轮趣科技（东莞）有限公司
品牌：WHEELTEC
官网：wheeltec.net
淘宝店铺：shop114407458.taobao.com 
速卖通: https://minibalance.aliexpress.com/store/4455017
版本：1.0
修改时间：2021-12-09

Brand: WHEELTEC
Website: wheeltec.net
Taobao shop: shop114407458.taobao.com 
Aliexpress: https://minibalance.aliexpress.com/store/4455017
Version:1.0
Update：2021-12-09

All rights reserved
***********************************************/
#include "timer.h"
#include "led.h"
/**************************************************************************
函数功能：定时中断初始化
入口参数：arr：自动重装值  psc：时钟预分频数 
返回  值：无
**************************************************************************/
void Timer1_Init(u16 arr,u16 psc)  // 49  7199 定时时间为1ms
{  
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	
	TIM_TimeBaseInitStruct.TIM_Period = arr;               //重装载值               49
	TIM_TimeBaseInitStruct.TIM_Prescaler = psc;            //预分频系数             7199
	TIM_TimeBaseInitStruct.TIM_ClockDivision =0;           //时钟分割
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//TIM向上计数模式    0
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStruct);
	
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);               //使能定时器中断
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM1_UP_IRQn;        //使能按键所在的外部中断通道
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;           //使能外部中断通道
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //抢占优先级1
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;        //响应优先级3
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_Cmd(TIM1,ENABLE);
}  


//该定时器的定时时间为1毫秒。

//定时器的时钟频率为72MHz，预分频系数为7199，重装载值为49。根据定时器的计算公式，定时时间可以计算如下：

//定时时间 = (重装载值 + 1) * (预分频系数 + 1) / 时钟频率

//定时时间 = (49 + 1) * (7199 + 1) / 72000000 ≈ 0.005秒 ≈ 5毫秒
