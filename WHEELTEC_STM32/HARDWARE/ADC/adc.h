#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"
 /**************************************************************************
作者：平衡小车之家 
淘宝店铺：http://shop114407458.taobao.com/
**************************************************************************/
#define Battery_Ch 6

extern int last_Adc_Average;
extern int last_Adc;
extern int total_Adc;
extern int circle; 

u16 Get_Adc(u8 ch);
u16 Get_Adc2(u8 ch);
int Get_battery_volt(void);   
void Adc_Init(void);  
u16 Get_Adc_Average(u8 ch,u8 times);
u16* Get_Adc_Array(u8 ch,u16 arr[], int size);
int medianFilter(u16 signal[], int size, int windowSize);
float Get_Adc_Average_Angle(float ADC);
float Get_Adc_Average_Speed(float ADC);
#endif 















