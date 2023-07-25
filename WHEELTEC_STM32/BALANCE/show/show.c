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
#include "show.h"
unsigned char i,temp;          //计数变量
unsigned char Send_Count; //串口需要发送的数据个数
float Vol;
extern u8 success_flag;
extern float D_Angle_Balance;
/**************************************************************************
函数功能：OLED显示
入口参数：无
返回  值：无
**************************************************************************/
void oled_show(void)
{
		//=============第1行显示角度PD控制P参数Position_KP=======================//	
//		OLED_ShowString(00,00,"M-P");                   
//		OLED_ShowNumber(40,00,Balance_KP,3,12);
//		OLED_ShowString(57,00,"."),  
//		OLED_ShowNumber(61,00,(int)(Balance_KP*10)%10,1,12);//motor_position

//		OLED_ShowString(95,00,"A:");	  
//		OLED_ShowNumber(108,00,Amplitude1,2,12);
		
    OLED_ShowString(0,00,"M-P:");  
		OLED_ShowFloat(45,00,motor_position,5,3,12);
	  OLED_ShowString(0,10,"M-S:");  
		OLED_ShowFloat(45,10,motor_velocity,5,3,12);
	  OLED_ShowString(0,20,"S-P:");  
		OLED_ShowFloat(45,20,sensor_position,5,3,12);
	  OLED_ShowString(0,30,"S-S:");  
		OLED_ShowFloat(45,30,sensor_velocity,5,3,12);
		//=============第2行显示角度PD控制D参数=======================//	
//		OLED_ShowString(00,10,"M-S");
//		OLED_ShowNumber(40,10,motor_velocity,3,12);
////		OLED_ShowString(57,10,"."),  
////		OLED_ShowNumber(61,10,(int)(motor_velocity*10)%10,1,12);

//		OLED_ShowString(95,10,"A:");	  
//		OLED_ShowNumber(108,10,Amplitude2,2,12);
		//=============第3行显示编码器Position_KP=======================//	
//		OLED_ShowString(00,20,"S-P");
//		OLED_ShowNumber(40,20,sensor_position,3,12);
//		OLED_ShowString(57,20,"."),  
//		OLED_ShowNumber(61,20,(int)(sensor_position*10)%10,1,12);
//		
//		OLED_ShowString(95,20,"A:");	  
//		OLED_ShowNumber(108,20,Amplitude3,2,12);
		//=============第4行显示编码器1=======================//	
//		OLED_ShowString(00,30,"S-S");
//		OLED_ShowNumber(40,30,sensor_velocity,3,12);
//		OLED_ShowString(57,30,"."),  
//		OLED_ShowNumber(61,30,(int)(sensor_velocity*10)%10,1,12);
//		
//		OLED_ShowString(95,30,"A:");	  
//		OLED_ShowNumber(108,30,Amplitude4,2,12);
		//======这是滚动菜单 选择需要调节的PD参数											
//		  if(Menu==1)
//	   	{
//			 OLED_ShowChar(75,00,'Y',12,1);   
//			 OLED_ShowChar(75,10,'N',12,1);
//			 OLED_ShowChar(75,20,'N',12,1);
//			 OLED_ShowChar(75,30,'N',12,1);
//	  	}
//		  else	if(Menu==2)
//	   	{
//			 OLED_ShowChar(75,00,'N',12,1);
//			 OLED_ShowChar(75,10,'Y',12,1);
//			 OLED_ShowChar(75,20,'N',12,1);
//			 OLED_ShowChar(75,30,'N',12,1);
//			}		
//      else if(Menu==3)
//	   	{			
//			 OLED_ShowChar(75,00,'N',12,1);
//			 OLED_ShowChar(75,10,'N',12,1);
//			 OLED_ShowChar(75,20,'Y',12,1);
//			 OLED_ShowChar(75,30,'N',12,1);
//			}		
//      else if(Menu==4)
//	   	{				
//			 OLED_ShowChar(75,00,'N',12,1);
//			 OLED_ShowChar(75,10,'N',12,1);
//			 OLED_ShowChar(75,20,'N',12,1);
//			 OLED_ShowChar(75,30,'Y',12,1);
//	 	  } 
			
	//=============第五行显示电压和目标位置=======================//			
			OLED_ShowString(80,40,"T:");	  
//			OLED_ShowNumber(95,40,Position_Zero,5,12) ; 
      int action_show = (int)(action * 100);
      OLED_ShowNumber(95,40,action_show,5,12) ; 
			OLED_ShowString(00,40,"VOL:");
			OLED_ShowString(41,40,".");
			OLED_ShowString(63,40,"V");
			OLED_ShowNumber(28,40,Voltage/100,2,12);
			OLED_ShowNumber(51,40,Voltage%100,2,12);
			
		 if(Voltage%100<10) 	OLED_ShowNumber(45,40,0,2,12);
		//=============第六行显示角位移传感器和编码器数据=======================//
		OLED_ShowString(80,50,"P:");    
		OLED_ShowNumber(95,50,Encoder,5,12); 
		OLED_ShowString(0,50,"ADC:"); // :=5  
		OLED_ShowNumber(30,50,Adc,4,12);
		//=============刷新=======================//
		OLED_Refresh_Gram();	
	}

/**************************************************************************
函数功能：虚拟示波器往上位机发送数据 关闭显示屏
入口参数：无
返回  值：无
作    者：平衡小车之家
**************************************************************************/
void DataScope(void)
{   
		DataScope_Get_Channel_Data( Adc, 1 );      
		DataScope_Get_Channel_Data( Encoder, 2 );         
		DataScope_Get_Channel_Data( 0, 3 );              
		DataScope_Get_Channel_Data(motor_position , 4 );   
		DataScope_Get_Channel_Data(motor_velocity, 5 ); //用您要显示的数据替换0就行了
		DataScope_Get_Channel_Data(sensor_position , 6 );//用您要显示的数据替换0就行了
		DataScope_Get_Channel_Data(sensor_velocity, 7 );
//		DataScope_Get_Channel_Data( 0, 8 ); 
//		DataScope_Get_Channel_Data(0, 9 );  
//		DataScope_Get_Channel_Data( 0 , 10);
//		Send_Count = DataScope_Data_Generate(3);
//		for( i = 0 ; i < Send_Count; i++) 
//		{
//		while((USART1->SR&0X40)==0);  
//		USART1->DR = DataScope_OutPut_Buffer[i]; 
//		}
}	

//提示页 ：检测到摆杆需要调节，请根据提示调节摆杆，按下一页按键开始调节
void show_Tips(void)
{
 //启动角位移安装检查模式，请根据提示操作。按下用户按键确认...
    OLED_ShowChinese_12(28,0,0,1);
	OLED_ShowChinese_12(42,0,1,1);
	OLED_ShowChinese_12(56,0,2,1);
	OLED_ShowChinese_12(70,0,3,1);
	OLED_ShowChinese_12(84,0,4,1);
	OLED_ShowChinese_12(98,0,5,1);
	OLED_ShowChinese_12(112,0,6,1);
	OLED_ShowChinese_12(0,16,7,1);
	OLED_ShowChinese_12(14,16,8,1);
	OLED_ShowChinese_12(28,16,9,1);
	OLED_ShowChinese_12(42,16,10,1);
	OLED_ShowChinese_12(56,16,11,1);
	OLED_ShowChinese_12(70,16,12,1);
	OLED_ShowChinese_12(84,16,13,1);
	OLED_ShowChinese_12(98,16,14,1);
	OLED_ShowChinese_12(112,16,15,1);
	OLED_ShowChinese_12(0,32,16,1);
	OLED_ShowChinese_12(14,32,17,1);
	OLED_ShowChinese_12(28,32,18,1);
	OLED_ShowChinese_12(42,32,19,1);
	OLED_ShowChinese_12(56,32,20,1);
	OLED_ShowChinese_12(70,32,21,1);
	OLED_ShowChinese_12(84,32,22,1);
	OLED_ShowChinese_12(98,32,23,1);
	OLED_ShowChinese_12(112,32,24,1);
	OLED_ShowChinese_12(0,48,25,1);
	OLED_ShowChinese_12(14,48,26,1);
	OLED_ShowChinese_12(28,48,27,1);
	OLED_ShowChinese_12(42,48,28,1);
	OLED_ShowChinese_12(46,48,28,1);
	OLED_ShowChinese_12(50,48,28,1);
	
	OLED_Refresh_Gram();
}

//第0页：请固定好倒立摆，使摆杆垂直向下并保持静止。安装完毕后按下一页按键继续
void step_0(void)
{
  OLED_ShowChinese_12(0,0,12,1);
	OLED_ShowChinese_12(14,0,29,1);
	OLED_ShowChinese_12(28,0,30,1);
	OLED_ShowChinese_12(42,0,31,1);
	OLED_ShowChinese_12(56,0,32,1);
	OLED_ShowChinese_12(70,0,33,1);
	OLED_ShowChinese_12(84,0,34,1);
	OLED_ShowChinese_12(98,0,11,1);
	OLED_ShowChinese_12(112,0,35,1);
	OLED_ShowChinese_12(0,16,36,1);
	OLED_ShowChinese_12(14,16,37,1);
	OLED_ShowChinese_12(28,16,38,1);
	OLED_ShowChinese_12(42,16,39,1);
	OLED_ShowChinese_12(56,16,40,1);
	OLED_ShowChinese_12(70,16,41,1);
	OLED_ShowChinese_12(84,16,42,1);
	OLED_ShowChinese_12(98,16,43,1);
	OLED_ShowChinese_12(112,16,44,1);
	OLED_ShowChinese_12(0,32,45,1);
	OLED_ShowChinese_12(14,32,46,1);
	OLED_ShowChinese_12(28,32,19,1);
	OLED_ShowChinese_12(42,32,47,1);
	OLED_ShowChinese_12(56,32,48,1);
	OLED_ShowChinese_12(70,32,49,1);
	OLED_ShowChinese_12(84,32,50,1);
	OLED_ShowChinese_12(98,32,51,1);
	OLED_ShowChinese_12(112,32,24,1);
	OLED_ShowChinese_12(0,48,41,1);
	OLED_ShowChinese_12(14,48,52,1);
	OLED_ShowChinese_12(28,48,53,1);
	OLED_ShowChinese_12(42,48,24,1);
	OLED_ShowChinese_12(56,48,25,1);
	OLED_ShowChinese_12(70,48,54,1);
	OLED_ShowChinese_12(84,48,55,1);
	OLED_ShowChinese_12(98,48,28,1);
	OLED_ShowChinese_12(102,48,28,1);
	OLED_ShowChinese_12(106,48,28,1);
	OLED_Refresh_Gram();
}

//第1页：把连接摆杆和角位移传感器最顶端的螺丝拧松，然后调节位于三颗螺丝中间的旋钮
void step_1(void)
{
  OLED_ShowChinese_12(0,0,66,1);
	OLED_ShowChinese_12(14,0,67,1);
	OLED_ShowChinese_12(28,0,68,1);
	OLED_ShowChinese_12(42,0,36,1);
	OLED_ShowChinese_12(56,0,37,1);
	OLED_ShowChinese_12(70,0,69,1);
	OLED_ShowChinese_12(84,0,2,1);
	OLED_ShowChinese_12(98,0,3,1);
	OLED_ShowChinese_12(112,0,4,1);
	OLED_ShowChinese_12(0,16,70,1);
	OLED_ShowChinese_12(14,16,71,1);
	OLED_ShowChinese_12(28,16,72,1);
	OLED_ShowChinese_12(42,16,73,1);
	OLED_ShowChinese_12(56,16,74,1);
	OLED_ShowChinese_12(70,16,75,1);
	OLED_ShowChinese_12(84,16,76,1);
	OLED_ShowChinese_12(98,16,77,1);
	OLED_ShowChinese_12(112,16,78,1);
	OLED_ShowChinese_12(0,32,79,1);
	OLED_ShowChinese_12(14,32,80,1);
	OLED_ShowChinese_12(28,32,11,1);
	OLED_ShowChinese_12(42,32,81,1);
	OLED_ShowChinese_12(56,32,51,1);
	OLED_ShowChinese_12(70,32,60,1);
	OLED_ShowChinese_12(84,32,61,1);
	OLED_ShowChinese_12(98,32,82,1);
	OLED_ShowChinese_12(112,32,83,1);
	OLED_ShowChinese_12(0,48,84,1);
	OLED_ShowChinese_12(14,48,85,1);
	OLED_ShowChinese_12(28,48,77,1);
	OLED_ShowChinese_12(42,48,78,1);
	OLED_ShowChinese_12(56,48,86,1);
	OLED_ShowChinese_12(70,48,87,1);
	OLED_ShowChinese_12(84,48,76,1);
	OLED_ShowChinese_12(98,48,88,1);
	OLED_ShowChinese_12(112,48,89,1);
	OLED_Refresh_Gram();
}

//第2页：使得摆杆垂直向下时ADC的值在1010至1030之间，然后拧紧最顶端的螺丝。调
void step_2(void)
{
  OLED_ShowChinese_12(0,0,35,1);
	OLED_ShowChinese_12(14,0,90,1);
	OLED_ShowChinese_12(28,0,36,1);
	OLED_ShowChinese_12(42,0,37,1);
	OLED_ShowChinese_12(56,0,38,1);
	OLED_ShowChinese_12(70,0,39,1);
	OLED_ShowChinese_12(84,0,40,1);
	OLED_ShowChinese_12(98,0,41,1);
	OLED_ShowChinese_12(112,0,91,1);

	OLED_ShowChinese_12(0,16,92,1);  /* A */
	OLED_ShowChinese_12(9,16,93,1);  /* D */
	OLED_ShowChinese_12(18,16,94,1); /* C */
	OLED_ShowChinese_12(27,16,76,1);
	OLED_ShowChinese_12(41,16,95,1);
	OLED_ShowChinese_12(55,16,57,1); //在

	OLED_ShowChinese_12(69,16,96,1); //1010
	OLED_ShowChinese_12(78,16,97,1);
	OLED_ShowChinese_12(87,16,96,1);
	OLED_ShowChinese_12(96,16,97,1); 
	
	OLED_ShowChinese_12(105,16,99,1);
	
	OLED_ShowChinese_12(0,32,96,1); //1030
	OLED_ShowChinese_12(9,32,97,1);
	OLED_ShowChinese_12(18,32,98,1);
	OLED_ShowChinese_12(27,32,97,1); 
	
	OLED_ShowChinese_12(36,32,100,1); 
	OLED_ShowChinese_12(50,32,87,1); 
	OLED_ShowChinese_12(64,32,11,1);
	OLED_ShowChinese_12(78,32,81,1);
	OLED_ShowChinese_12(92,32,51,1);
	OLED_ShowChinese_12(106,32,79,1);
	OLED_ShowChinese_12(0,48,101,1);
	OLED_ShowChinese_12(14,48,73,1);
	OLED_ShowChinese_12(28,48,74,1);
	OLED_ShowChinese_12(42,48,75,1);
	OLED_ShowChinese_12(56,48,76,1);
	OLED_ShowChinese_12(70,48,77,1);
	OLED_ShowChinese_12(84,48,78,1);
	OLED_ShowChinese_12(98,48,19,1);
	OLED_ShowChinese_12(112,48,60,1);
	OLED_Refresh_Gram();
}


//第3页：节完成后按下用户按键开始检查
void step_3(void)
{

	OLED_ShowChinese_12(0,0,61,1);
	OLED_ShowChinese_12(14,0,49,1);
	OLED_ShowChinese_12(28,0,102,1);
	OLED_ShowChinese_12(42,0,51,1);
	OLED_ShowChinese_12(56,0,20,1);
	OLED_ShowChinese_12(70,0,21,1);
	OLED_ShowChinese_12(84,0,22,1);
	OLED_ShowChinese_12(98,0,23,1);
	OLED_ShowChinese_12(112,0,24,1);
	OLED_ShowChinese_12(0,16,25,1);
	OLED_ShowChinese_12(14,16,103,1);
	OLED_ShowChinese_12(28,16,104,1);
	OLED_ShowChinese_12(42,16,7,1);
	OLED_ShowChinese_12(56,16,8,1);
	OLED_ShowChinese_12(70,16,28,1);
	OLED_ShowChinese_12(74,16,28,1);
	OLED_ShowChinese_12(78,16,28,1);
	OLED_ShowChar(24,40,'A',16,1);
	OLED_ShowChar(32,40,'D',16,1);
	OLED_ShowChar(40,40,'C',16,1);
  OLED_ShowNumber(78,40,Adc,4,16);
	OLED_Refresh_Gram();
}

//系统检查页：正在检查调节是否正确........10     请保持摆杆静止
void check_display(void)
{
	OLED_ShowChinese_12(21,0,56,1);
	OLED_ShowChinese_12(35,0,57,1);
	OLED_ShowChinese_12(49,0,58,1);
	OLED_ShowChinese_12(63,0,59,1);
	OLED_ShowChinese_12(77,0,60,1);
	OLED_ShowChinese_12(91,0,61,1);
	OLED_ShowChinese_12(105,0,62,1);
	OLED_ShowChinese_12(0,16,63,1);
	OLED_ShowChinese_12(14,16,64,1);
	OLED_ShowChinese_12(28,16,65,1);
	OLED_ShowChinese_12(46,16,28,1);
	OLED_ShowChinese_12(53,16,28,1);
	OLED_ShowChinese_12(60,16,28,1);
	OLED_ShowChinese_12(67,16,28,1);
	OLED_ShowChinese_12(74,16,28,1);
	OLED_ShowChinese_12(81,16,28,1);
	//倒计时
	//请保持摆杆静止
	OLED_ShowChinese_12(14,32,12,1);
	OLED_ShowChinese_12(28,32,43,1);
	OLED_ShowChinese_12(42,32,44,1);
	OLED_ShowChinese_12(56,32,36,1);
	OLED_ShowChinese_12(70,32,37,1);
	OLED_ShowChinese_12(84,32,45,1);
	OLED_ShowChinese_12(98,32,46,1);
	OLED_ShowChar(24,48,'A',16,1);
	OLED_ShowChar(32,48,'D',16,1);
	OLED_ShowChar(40,48,'C',16,1);
  OLED_ShowNumber(78,48,Adc,4,16);
	OLED_Refresh_Gram();
}

//调节成功页：调节成功  按下用户按键进入系统
void success_display(void)
{
	OLED_ShowChinese(32,4,0,1);      /*调*/
	OLED_ShowChinese(48,4,1,1);      /*节*/
	OLED_ShowChinese(64,4,2,1);      /*成*/
	OLED_ShowChinese(80,4,3,1);      /*功*/
	OLED_ShowChinese(24,24,4,1);     /*按*/
	OLED_ShowChinese(40,24,5,1);     /*下*/
	OLED_ShowChinese(56,24,6,1);     /*用*/
	OLED_ShowChinese(72,24,7,1);     /*户*/
	OLED_ShowChinese(88,24,8,1);     /*按*/
	OLED_ShowChinese(104,24,9,1);    /*键*/
	OLED_ShowChinese(0,44,10,1);     /*进*/
	OLED_ShowChinese(16,44,11,1);    /*入*/
	OLED_ShowChinese(32,44,12,1);    /*系*/
	OLED_ShowChinese(48,44,13,1);	   /*统*/
	OLED_Refresh_Gram();
}

//调节失败页：	检查不通过  请重新调节
void fail_display(void)
{
	OLED_ShowChinese(24,16,14,1);
	OLED_ShowChinese(40,16,15,1);
	OLED_ShowChinese(56,16,16,1);
	OLED_ShowChinese(72,16,17,1);
	OLED_ShowChinese(88,16,18,1);
	OLED_ShowChinese(24,32,19,1);
	OLED_ShowChinese(40,32,20,1);
	OLED_ShowChinese(56,32,21,1);
	OLED_ShowChinese(72,32,0,1);
	OLED_ShowChinese(88,32,1,1);
	OLED_Refresh_Gram();
}
