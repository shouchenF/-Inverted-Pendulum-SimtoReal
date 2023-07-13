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
#include "adc.h"
/**************************************************************************
函数功能：ADC初始化
入口参数：无
返回  值：无
作    者：平衡小车之家
**************************************************************************/
int last_Adc_Average = 0;
int last_Adc = 0;
int total_Adc = 0;
int circle = 0; 


void Adc_Init(void)
{    
 	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2	, ENABLE );	  //使能ADC1通道时钟
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M
	//设置模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	//ADC1
	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	ADC_ResetCalibration(ADC1);	//使能复位校准  	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束	
	ADC_StartCalibration(ADC1);	 //开启AD校准
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
	
	//ADC2
	ADC_DeInit(ADC2);  //复位ADC2,将外设 ADC2 的全部寄存器重设为缺省值
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC2, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   
	ADC_Cmd(ADC2, ENABLE);	//使能指定的ADC2
	ADC_ResetCalibration(ADC2);	//使能复位校准  	 
	while(ADC_GetResetCalibrationStatus(ADC2));	//等待复位校准结束	
	ADC_StartCalibration(ADC2);	 //开启AD校准
	while(ADC_GetCalibrationStatus(ADC2));	 //等待校准结束
}		

/**************************************************************************
函数功能：AD采样
入口参数：ADC1 的通道
返回  值：AD转换结果
作    者：平衡小车之家
**************************************************************************/
u16 Get_Adc(u8 ch)   
{
	//设置指定ADC的规则组通道，一个序列，采样时间
	if(ch==6) ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	     //电源电压通过，保证准确性，选择最大周期
	else if (ch==3)   ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_1Cycles5 ); //角位移传感器通道，保证采样速度，选择最小周期			     
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		       //使能指定的ADC1的软件转换启动功能		 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));  //等待转换结束
	return ADC_GetConversionValue(ADC1);	           //返回最近一次ADC1规则组的转换结果
}

u16 Get_Adc2(u8 ch)   
{
	//设置指定ADC的规则组通道，一个序列，采样时间
	if(ch==6) ADC_RegularChannelConfig(ADC2, ch, 1, ADC_SampleTime_239Cycles5 );	     //电源电压通过，保证准确性，选择最大周期
	else if (ch==3)   ADC_RegularChannelConfig(ADC2, ch, 1, ADC_SampleTime_1Cycles5 ); //角位移传感器通道，保证采样速度，选择最小周期			     
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);		       //使能指定的ADC1的软件转换启动功能		 
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC ));  //等待转换结束
	return ADC_GetConversionValue(ADC2);	           //返回最近一次ADC1规则组的转换结果
}

/**************************************************************************
函数功能：读取电池电压 
入口参数：无
返回  值：电池电压 单位MV
作    者：平衡小车之家
**************************************************************************/
int Get_battery_volt(void)   
{  
	int Volt;                                          //电池电压
	Volt=Get_Adc2(Battery_Ch)*3.3*11*100/1.0/4096;		 //电阻分压，具体根据原理图简单分析可以得到
	return Volt;
}


//获取通道ch的转换值，取times次,然后平均 
//ch:通道编号
//times:获取次数
//返回值:通道ch的times次转换结果平均值
u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_us(200);
	}
	return temp_val/times;
}

u16* Get_Adc_Array(u8 ch,u16 arr[], int size)
{
	u8 t;
	for(t=0;t<size;t++)
	{
		arr[t]=Get_Adc(ch);
		delay_us(200);
	}
	return arr;
} 

int medianFilter(u16 signal[], int size, int windowSize) {
    int result;

    // 窗口大小必须为奇数
    if (windowSize % 2 == 0 || windowSize <= 1) {
        printf("窗口大小必须为奇数且大于1。\n");
        return -1; // 返回错误码
    }

    int halfWindow = windowSize / 2;
    int window[windowSize];

    for (int i = halfWindow; i < size - halfWindow; i++) {
        // 提取窗口内的采样数据
        for (int j = 0; j < windowSize; j++) {
            window[j] = signal[i + j - halfWindow];
        }

        // 对窗口内的采样数据进行排序
        for (int j = 0; j < windowSize - 1; j++) {
            for (int k = 0; k < windowSize - j - 1; k++) {
                if (window[k] > window[k + 1]) {
                    int temp = window[k];
                    window[k] = window[k + 1];
                    window[k + 1] = temp;
                }
            }
        }

        // 取排序后的中间值作为滤波结果
        result = window[windowSize / 2];
    }

    return result;
}

// 角位移传感器的角度	 
float Get_Adc_Average_Angle(float ADC)
{
	
	float Adc_Average;
	int Adc_Angle;
 
	Adc_Average = ADC;
	
	Adc_Angle = (float)Adc_Average/4096*360;
	
	if(Adc_Angle > last_Adc)
	{
		if(Adc_Angle > last_Adc && Adc_Angle == 350)
		{
			total_Adc = Adc_Angle + 360*(circle++);
			last_Adc = Adc_Angle;
			return total_Adc;
		}
		total_Adc = Adc_Angle + 360*circle;
	}
	else if(Adc_Angle < last_Adc)
	{		
		if(Adc_Angle < last_Adc && Adc_Angle == 350)
		{
			--circle;
		}
		total_Adc = Adc_Angle + 360*circle;
	}
	last_Adc = Adc_Angle;
	return total_Adc;
}
// 角位移传感器的速度
float Get_Adc_Average_Speed(float ADC)
{

	float Adc_Average;
	float Adc_Speed;

	Adc_Average = ADC;
	Adc_Speed = (float)(Adc_Average-last_Adc_Average)/4096*360*200;
	last_Adc_Average = Adc_Average;
	return (float)Adc_Speed;
}







