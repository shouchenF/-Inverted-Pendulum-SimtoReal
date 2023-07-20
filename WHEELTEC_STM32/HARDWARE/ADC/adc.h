#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"
#include "control.h"
 /**************************************************************************
作者：平衡小车之家 
淘宝店铺：http://shop114407458.taobao.com/
**************************************************************************/
#define Battery_Ch 6

extern int adc_c;		
extern int adc_pre;
extern float angle_pre;
extern float angle_c;
extern float angle_total;
extern float angle_total_pre;
extern int circle;
extern float speed;	

u16 Get_Adc(u8 ch);
u16 Get_Adc2(u8 ch);
int Get_battery_volt(void);   
void Adc_Init(void);  
u16 Get_Adc_Average(u8 ch,u8 times);
u16* Get_Adc_Array(u8 ch,u16 arr[], int size);
int medianFilter(u16 signal[], int size, int windowSize);
float Get_Adc_Average_Angle(float ADC);
float Get_Adc_Average_Speed(void);
#endif 















