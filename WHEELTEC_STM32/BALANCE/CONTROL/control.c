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
#include "control.h"
#include <math.h>
#include <string.h>
#include <usart.h>
#define LPF(x, f, c) (c*x + (1-c)*f)
float pc_fil, vc_fil, ec_fil, wc_fil;
extern float motor_position, motor_velocity;
int Balance_Pwm,Position_Pwm; //目标角度PWM、目标位置PWM
u8 Position_Target;//用于标记位置控制的时间
u8 Swing_up=1; //用于标记手动起摆时，是否是第一次进入手动起摆函数

// 将变量转换为字符串形式
char data_str[200];
unsigned char byteArry1[sizeof(double)];
unsigned char byteArry2[sizeof(double)];
unsigned char byteArry3[sizeof(double)];
unsigned char byteArry4[sizeof(double)];
uint8_t data[3]={0x12,0x04,0x06};
uint8_t data_array[20];
//倾角PD控制所用到的参数
float Bias;                       //倾角偏差
float Last_Bias,D_Bias;    //PID相关变量
int balance;                      //PWM返回值 

//位置PD控制所用到的参数
float Position_PWM,Last_Position,Position_Bias,Position_Differential;
float Position_Least;
int my_Position(float pt,float pc);
u8 auto_run=0; //手动起摆或自动起摆标志位，默认是手动起摆
u8 autorun_step0=0; //自动起摆第0步，找到中点，等待起摆
u8 autorun_step1=1; //自动起摆第1步
u8 autorun_step2=0; //自动起摆第2步
long Target_Position;//目标位置
float D_Angle_Balance; //摆杆角度变化率
long success_count=0;//摆杆在平衡位置的成功次数记录
u8 success_flag=0; //自动起摆时满足平衡次数可以起摆的标志位
long wait_count=0; //等待计数，计时时间到后，获取起摆成功的位置
long D_Count;//用于辅助获取摆杆角度变化率的中间变量
float Last_Angle_Balance; //用于获取摆杆角度变化率函数中，保存上一次角度
float pt = 0;
u8 left,right;
float error = 0.0, error_sum = 0.0, Velocity_PWM = 0.0;
// unsigned char byteArry[sizeof(float)];
u16 arr[20];

#define _bndf(x,m,M) fmin(fmax(x,m),M)
/**************************************************************************
函数功能：所有的控制代码都在这里面
          TIM1控制的5ms定时中断 
**************************************************************************/
int TIM1_UP_IRQHandler(void)  
{    
	if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 //5ms定时中断
	{   
	 TIM_ClearITPendingBit(TIM1, TIM_IT_Update  );             //===清除定时器1中断标志位	                     
		 if(delay_flag==1)
		 {
			 if(++delay_50==10)	 delay_50=0,delay_flag=0;          //===给主函数提供50ms的精准延时  10次*5ms = 50ms
		 }		
		Encoder=Read_Encoder(4);             	                   //===更新编码器位置信息	 
		Adc=Get_Adc_Average(3,10);                               //===更新姿态
		 
//		Get_Adc_Array(3,arr,10);
//		Angle_Balance = medianFilter(arr, 15, 15);	 
//  	Get_D_Angle_Balance();                                   //===获得摆杆角速度

	 
/************状态：电机速度、位置和角位移传感器的速度、位置*****************/	 
//  字符串收发：已调试成功
		motor_position = Read_Encoder_Angle(Encoder);
		sensor_position = Get_Adc_Average_Angle(Adc);
		motor_velocity = Read_Encoder_Speed(Encoder);
		sensor_velocity = Get_Adc_Average_Speed();
		pc_fil = LPF(motor_position, pc_fil,0.2f);
		vc_fil = LPF(motor_velocity, vc_fil,0.2f);
		ec_fil = LPF(sensor_position,ec_fil,0.2f);
		wc_fil = LPF(sensor_velocity,wc_fil,0.2f);
		 
/************** 串口发送数据方式一：使用字符串传输数据（整型和浮点型） **********************/				 
//		sprintf(data_str, "%-8.4f, %-8.4f, %-8.4f, %-8.4f\n", pc_fil, ec_fil, vc_fil, wc_fil);
//		Usart_SendString(USART1, data_str);
//		motor_velocity = 0.123;
		
/************** 串口发送数据方式二：传输数据打包-16进制传输（整型） **********************/		
//		data_array[0] =  0x12;
//		data_array[1] =  0x34;
//		data_array[2] = (int)Encoder & 0xFF;
//		data_array[3] = ((int)Encoder >> 8) & 0xFF;
//		data_array[4] = (int)Adc & 0xFF;
//		data_array[5] = ((int)Adc >> 8) & 0xFF;
//		data_array[6] =  0x56;
//		data_array[7] =  0x78;
//		usart1_dma_tx_data(data_array, 8);
//		for(uint8_t  i = 0 ; i < 8; i++)
//			{
//				USART_SendData(USART1, *(data_array + i));
//        while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);  
//			}
	

/************** 串口发送数据方式三： 串口传输数据打包-16进制传输（整型和浮点型） **********************/		
//    FloatToByte(pc_fil, byteArry1); // 8个字节数据
//		FloatToByte(vc_fil, byteArry2); // 8个字节数据
//		data_array[0] =  0x12;  // 帧头1
//		data_array[1] =  0x34;  // 帧头2
//		data_array[2] = byteArry1[0];   // 电机位置低字节
//		data_array[3] = byteArry1[1];   // 电机位置高字节
//		data_array[4] = byteArry1[2];   // 电机位置低字节
//		data_array[5] = byteArry1[3];   // 电机位置高字节
//		/*电机速度为浮点型数据，将其十进制数转换为单精度浮点数是4个字节（32位），转换网站：http://www.styb.cn/cms/ieee_754.php*/
//		data_array[6] = byteArry2[0];							// 电机速度低字节		
//		data_array[7] = byteArry2[1];				// 电机速度高字节	
//		data_array[8] = byteArry2[2];							// 电机速度低字节		
//		data_array[9] = byteArry2[3];				// 电机速度高字节	
//		data_array[10] =  0x56;  // 帧尾1
//		data_array[11] =  0x78;  // 帧尾2
//		usart1_dma_tx_data(data_array, 12);
//		for(uint8_t  i = 0 ; i < sizeof(data_array); i++)
//			{
//				USART_SendData(USART1, *(data_array + i));
//        while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);  
//			}

/************** 串口发送数据方式三： 串口传输数据打包-16进制传输（电机和编码器信息浮点型） **********************/		
    FloatToByte(pc_fil, byteArry1); // 8个字节数据
		FloatToByte(vc_fil, byteArry2); // 8个字节数据
		FloatToByte(ec_fil, byteArry3); // 8个字节数据
		FloatToByte(wc_fil, byteArry4); // 8个字节数据
		data_array[0] =  0x12;  // 帧头1
		data_array[1] =  0x34;  // 帧头2
		
		data_array[2] = byteArry1[0];   // 电机位置低字节
		data_array[3] = byteArry1[1];   // 电机位置高字节
		data_array[4] = byteArry1[2];   // 电机位置低字节
		data_array[5] = byteArry1[3];   // 电机位置高字节
		
		/*电机速度为浮点型数据，将其十进制数转换为单精度浮点数是4个字节（32位），转换网站：http://www.styb.cn/cms/ieee_754.php*/
		data_array[6] = byteArry2[0];							// 电机速度低字节		
		data_array[7] = byteArry2[1];				// 电机速度高字节	
		data_array[8] = byteArry2[2];							// 电机速度低字节		
		data_array[9] = byteArry2[3];				// 电机速度高字节	
		
		data_array[10] = byteArry3[0];							// 电机速度低字节		
		data_array[11] = byteArry3[1];				// 电机速度高字节	
		data_array[12] = byteArry3[2];							// 电机速度低字节		
		data_array[13] = byteArry3[3];				// 电机速度高字节	
		
		data_array[14] = byteArry4[0];							// 电机速度低字节		
		data_array[15] = byteArry4[1];				// 电机速度高字节	
		data_array[16] = byteArry4[2];							// 电机速度低字节		
		data_array[17] = byteArry4[3];				// 电机速度高字节	
		
		data_array[18] =  0x56;  // 帧尾1
		data_array[19] =  0x78;  // 帧尾2
		usart1_dma_tx_data(data_array, 20);
//		for(uint8_t  i = 0 ; i < sizeof(data_array); i++)
//			{
//				USART_SendData(USART1, *(data_array + i));
//        while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);  
//			}
			
/**************串口发送数据方式四： 仅发送浮点型小数 **********************/
//     FloatToByte(motor_velocity, byteArry); // 8个字节数据
//     for(uint8_t  i = 0 ; i < sizeof(float); i++)
//			{
//				USART_SendData(USART1, *(byteArry + i));
//        while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);  
//			}


		Moto = action;
		float a = _bndf(action,-0.7f,0.7f);
//		Moto = my_Position(a,pc_fil);
    Moto = my_velocity(a ,vc_fil);
//		Xianfu_Pwm();		 
		Set_Pwm(Moto);
	//自动起摆步骤1中的滑块边缘保护
//		if(Encoder>10000||Encoder<=6100)
//			Set_Pwm(0);	
	}
  Voltage=Get_battery_volt();           //===获取电池电压	      
	Key();                                //===扫描按键变化    	
	return 0;	  
} 

/**************************************************************************
函数功能：将一个浮点数转换为字节数组   倒序  大小端的问题
入口参数：浮点数   字节数组
**************************************************************************/
void FloatToByte(float floatNum, unsigned char* byteArry) {
    char* pchar = (char*)&floatNum;
    for (int i = 0; i < sizeof(float); i++) {
        *byteArry = *pchar;
        pchar++;
        byteArry++;
    }
}

/**************************************************************************
函数功能：将一个字节数组转换为浮点数
入口参数：  字节数组
**************************************************************************/
float Byte2Float(unsigned char* byteArry) {
    float floatNum;
    memcpy(&floatNum, byteArry, sizeof(float));
    return floatNum;
}
/**************************************************************************
函数功能：倾角PD控制
入口参数：角度
返回  值：倾角控制PWM
作    者：平衡小车之家
**************************************************************************/
 int Balance(float Angle)
 {  
 	 Bias=Angle-ZHONGZHI;              //求出平衡的角度中值 和机械相关
 	 D_Bias=Bias-Last_Bias;            //求出偏差的微分 进行微分控制
 	 balance=-Balance_KP*Bias-D_Bias*Balance_KD;   //===计算倾角控制的电机PWM  PD控制
   Last_Bias=Bias;                   //保持上一次的偏差
 	 return balance;
 }

/**************************************************************************
函数功能：位置PD控制 
入口参数：编码器
返回  值：位置控制PWM
作    者：平衡小车之家
**************************************************************************/
 int Position(int Encoder)
 {  
   	Position_Least =Encoder-Position_Zero;             //===
     Position_Bias *=0.8;		   
     Position_Bias += Position_Least*0.2;	             //===一阶低通滤波器  
 	  Position_Differential=Position_Bias-Last_Position;
 	  Last_Position=Position_Bias;
 		Position_PWM=Position_Bias*Position_KP+Position_Differential*Position_KD; //===速度控制		
 //    Position_PWM=Position_Bias*(Position_KP+Basics_Position_KP)/2+Position_Differential*(Position_KD+Basics_Position_KD)/2; //===位置控制	
 	  return Position_PWM;
 }
 
  int my_Position(float pt,float pc)
 {  
	 float pidC = 4080.0/0.44f;
   	Position_Least = pc;                               //===
     Position_Bias *=0.8;		   
     Position_Bias += Position_Least*0.2;	             //===一阶低通滤波器  
 	  Position_Differential=Position_Bias-Last_Position;
 	  Last_Position=Position_Bias;
 		Position_PWM=(pt-Position_Bias)*Position_KP*pidC - Position_Differential*Position_KD*pidC; //===速度控制		
 //    Position_PWM=Position_Bias*(Position_KP+Basics_Position_KP)/2+Position_Differential*(Position_KD+Basics_Position_KD)/2; //===位置控制	
 	  return (int)Position_PWM;
 }
 
int my_velocity(float target_velocity, float current_velocity)
{

  float pidC = 4080.0/0.44f;
	float kp=3.0, ki= 0.13;
	error = target_velocity - current_velocity;
	error_sum += error;
	
	// 设置积分项的上限和下限
  float ki_max = 7200.0f/ki; // 积分项的上限
  float ki_min = -7200.0f/ki; // 积分项的下限

  // 对积分项进行限制
  if (error_sum > ki_max) {
    error_sum = ki_max;
  } else if (error_sum < ki_min) {
    error_sum = ki_min;
  }
	
  Velocity_PWM =  kp * error *pidC + ki * error_sum *pidC; 
	if (Velocity_PWM > 7200.0f) {
    Velocity_PWM = 7200.0f;
  } else if (Velocity_PWM < -7200.0f) {
    Velocity_PWM = -7200.0f;
  }
	return (int)Velocity_PWM; // 0-7200
}
/**************************************************************************
函数功能：赋值给PWM寄存器
入口参数：PWM
返回  值：无
**************************************************************************/
void Set_Pwm(int moto)
{
    	if(moto<0)			BIN2=1,			BIN1=0;
			else 	          BIN2=0,			BIN1=1;
			PWMB=myabs(moto);
}

/**************************************************************************
函数功能：限制PWM赋值 
入口参数：无
返回  值：无
**************************************************************************/
void Xianfu_Pwm(void)
{	
	  int Amplitude=6900;    //===PWM满幅是7200 限制在6900
	  if(Moto<-Amplitude) Moto=-Amplitude;	
		if(Moto>Amplitude)  Moto=Amplitude;		
}
/**************************************************************************
函数功能：按键修改控制摆杆的位置
入口参数：无
返回  值：无
**************************************************************************/
void Key(void)
{	
	// 产品使用的是13线霍尔传感器电机，20减速比，程序上用的是4倍频，电机转一圈编码器上的数值是13*4*20 = 1040 
	int position=1040; //目标位置 电机原始位置是10000  转一圈是1040 和编码器精度有关，默认是摆杆Z轴转一圈，输出1040个跳变沿
	static int tmp,flag,count;
	tmp=click_N_Double(100); 
	
	if(tmp==1)flag=1;//++
  if(tmp==2)flag=2;//--
	
	if(flag==1) //摆杆向正方向运动
	{
		Position_Zero+=4;
		count+=4;	
		if(count==position) 	flag=0,count=0;
	}	
		if(flag==2) //摆杆向反方向运动
	{
		Position_Zero-=4;
		count+=4;	
		if(count==position) 	flag=0,count=0;
	}
	
	if(Long_Press_KEY2()==1)
	{
		Position_Zero=10000;
		auto_run=!auto_run;
	}
	
	if(Flag_Stop==1)
	{
		if(auto_run==1)
		{
			//提示信息
			//自动起摆模式
			LED=0;
		}
		
	else if(auto_run==0) LED=1;
	}

}

/**************************************************************************
函数功能：异常关闭电机
入口参数：电压
返回  值：1：异常  0：正常
**************************************************************************/
u8 Turn_Off(int voltage)
{
	    u8 temp; 
			if(1==Flag_Stop) //电池电压过低，关闭电机
			{	      
      Flag_Stop=1;				
      temp=1;                                            
			BIN1=0;                                            
			BIN2=0;
      }
			else
      temp=0;
				
			if(!(Adc>(ZHONGZHI-500)&&Adc<(ZHONGZHI+500))||(voltage<700))
			{
				Flag_Stop=1;
				temp=1;
			}

      return temp;			
}

/**************************************************************************
函数功能：绝对值函数
入口参数：int
返回  值：unsigned int
**************************************************************************/
int myabs(int a)
{ 		   
	  int temp;
		if(a<0)  temp=-a;  
	  else temp=a;
	  return temp;
}

/**************************************************************************
函数功能：零位标定
入口参数：无
返回  值：无
**************************************************************************/
void Find_Zero(void)
{
	 static float count;
	
	//初始化时 autorun_step0=0，自然会执行
if(autorun_step0==0)  //回到中位
 {
		Position_Zero=POSITION_MIDDLE;//设置目标中间位置

	 
	 //顺摆pid控制，让摆杆尽快稳定下来（目标角度是起始角，目标位置是起始位置）
		Balance_Pwm=Balance(Adc+2070)/8;   //倾角PD控制						
		Position_Pwm=Pre_Position(Encoder);
 
	 //如果偏离了中点太多，开始PID控制缓慢调整
		if(Encoder<7950)Moto=Incremental_PI(Encoder,POSITION_MIDDLE); //位置闭环控制;//离开边缘地方再开始PID控制
		else	Moto=-Balance_Pwm+Position_Pwm;     //读取PD控制器输出PWM

 //判断角度和位置是否在原始位置，如果 检测到200次 在原始位置，即可以等待起摆 
 //等待起摆：把刚刚用过的标志位、电机pwm值全部清零，更新Target_Position=POSITION_MIDDL-668，这是自起摆时的第一个目标点，使autorun_step0=1不要再进入这个函数
		if(Adc<(ANGLE_ORIGIN+300)&&Adc>(ANGLE_ORIGIN-300)&&(Encoder>(POSITION_MIDDLE-50)&&Encoder<(POSITION_MIDDLE+50)))  count++;
		if(Adc<(ANGLE_ORIGIN+300)&&Adc>(ANGLE_ORIGIN-300))count+=0.1;
		if(count>200)	autorun_step0=1,autorun_step1=0,Moto=0,Target_Position=POSITION_MIDDLE-668;//摆杆运动到中间位置，停止 //设置目标位置，准备甩杆
	 }
	 
	 //对电机速度限幅，防止电机闭环位置控制过快
		if(Moto>2500)	Moto=2500;   //控制位置闭环控制过程的速度
		if(Moto<-2500)Moto=-2500;
		Set_Pwm(Moto);//控制电机
 }		
/**************************************************************************
函数功能：自动起摆
入口参数：无
返回  值：无
**************************************************************************/
void Auto_run(void)
{
	static u8 speed=0;
	static u8 help_count=0;
	static u16 pid_adjust=0;

	if(autorun_step1==0)  //自动起摆第一步   （第一次执行，一定是进入这里）
	{
			 //判断应该往哪一边换向						
			if((Adc>(ANGLE_ORIGIN-120)&&Adc<(ANGLE_ORIGIN+120)))
			{
				if(D_Angle_Balance<=0) right=1;
				else if(D_Angle_Balance>0) left=1;
			}	

			//判断当摆杆回到初始位置时应该给出速度和位移
			if(left==1)
			{
				if((Adc>(ANGLE_ORIGIN-50)&&Adc<(ANGLE_ORIGIN+50)))
				{
					left=0;
					Target_Position=POSITION_MIDDLE+800;
					if(speed>1) Target_Position=POSITION_MIDDLE+160; //让摆杆缓慢调节直至满足稳摆
				}
			}	

			else if(right==1)
			{
				if((Adc>(ANGLE_ORIGIN-50)&&Adc<(ANGLE_ORIGIN+50)))
				{
					right=0;
					Target_Position=POSITION_MIDDLE-482;
					if(speed>1) Target_Position=POSITION_MIDDLE-160; //让摆杆缓慢调节直至满足稳摆
				}
			}							
			
		   //位置闭环控制
	  	 Moto=Position_PID(Encoder,Target_Position); 
			
		 //摆杆已经到达过平衡点附近，开始缓慢调节阶段。
			if(Adc<(ANGLE_MIDDLE+385)&&Adc>(ANGLE_MIDDLE-385)) 
			{
				speed++;
			}

			//判断当前情况是否符合起摆：起摆要素：位置不在边缘、角度在平衡点附近、角速度接近于0
			if(Adc<(ANGLE_MIDDLE+120)&&Adc>(ANGLE_MIDDLE-120)&&(Encoder>6300&&Encoder<9300)&&(D_Angle_Balance>-30&&D_Angle_Balance<30))
			{
				speed++;
				success_count++;		
			}
       
       else success_count = 0;//如果没在条件范围内，则需要重新清零等待下次判断

			//满足3次起摆情况，标记可以成功起摆，并给出起摆瞬间的pid
			if(success_count>3)
			{ 
				autorun_step1=1,success_flag=1;
				Balance_KP=210,Balance_KD=150,Position_KP=8,Position_KD=130;//自动起摆瞬间的pid参数
			}
	
			//限幅
			if(Moto>4100)	Moto=4100;   //控制位置闭环控制过程的速度
			if(Moto<-5100)Moto=-5100;	
	}
				
		//满足起摆条件，正常起摆
		else if(success_flag==1)//到最高点，起摆
		{	
			
			 if(wait_count==0) Position_Zero=Encoder;//起摆前，先获取当前位置作为平衡的目标点
			 Balance_Pwm =Balance(Adc);                                          //===角度PD控制	
			 if(++Position_Target>4)   Position_Pwm=Position(Encoder),Position_Target=0;    //===位置PD控制 25ms进行一次位置控制
			
			
			//起摆一段时间后，让摆杆缓慢恢复到中间点
			 wait_count++;
			 if(wait_count>100&&wait_count<2000) 
			 {
				 if(Position_Zero>8100) Position_Zero--;
				 else if(Position_Zero<8100)Position_Zero++;
			 }
			 if(wait_count>2000) wait_count = 2001;//锁住计数器值防止溢出循环
			 
			 //起摆后调节pid阶段	 起摆瞬间与稳摆需要的pid参数不一样 所以当起摆成功进入稳摆后，即可以把pid参数调回稳摆时的数值				 
			 if(help_count==0)
			 {
				 pid_adjust ++ ;
				 if( pid_adjust%100==0) //缓慢调节 避免出现震荡失稳
				 {
					 Balance_KP+=10;
					 Balance_KD+=10;
					 Position_KP += 0.5 ;
					 Position_KD +=10 ;
				 }
				 if(Balance_KP>400) Balance_KP=400;
				 if(Balance_KD>400) Balance_KD=400;
				 if(Position_KP>20) Position_KP=20;
				 if(Position_KD>300)Position_KD=300;
				 if(Balance_KP==400&&Balance_KD==400&&Position_KP==20&&Position_KD==300) help_count=1; //参数调节完毕后，释放pid数值限幅，此时用户可以通过按键改变pid参数
			 }		 

				//融合输出，最终效果
				Moto=Balance_Pwm-Position_Pwm;     //读取PD控制器输出PWM
		}
}
/**************************************************************************
函数功能：增量PI控制器
入口参数：编码器测量值，目标速度
返回  值：电机PWM
根据增量式离散PID公式 
pwm+=Kp[e（k）-e(k-1)]+Ki*e(k)+Kd[e(k)-2e(k-1)+e(k-2)]
e(k)代表本次偏差 
e(k-1)代表上一次的偏差  以此类推 
pwm代表增量输出
在我们的速度控制闭环系统里面，只使用PI控制
pwm+=Kp[e（k）-e(k-1)]+Ki*e(k)
**************************************************************************/
int Incremental_PI (int Encoder,int Target)
{ 	
	 static float Bias,Pwm,Last_bias;
	 Bias=Encoder-Target;                                  //计算偏差
	 Pwm+=10*(Bias-Last_bias)/20+10*Bias/20;   //增量式PI控制器
	 Last_bias=Bias;	                                     //保存上一次偏差 
	 return Pwm;                                           //增量输出
}

/**************************************************************************
函数功能：获取摆杆角度变化率
入口参数：无
返回  值：无
**************************************************************************/
void Get_D_Angle_Balance(void)
{
		if(++D_Count>5) //获取角度变化率，差分 时间常数25ms
		{
			D_Angle_Balance=Mean_Filter(Adc-Last_Angle_Balance);	//平滑滤波得到噪声更小的摆杆角速度信息		
//			D_Angle_Balance=Angle_Balance-Last_Angle_Balance;	//得到摆杆角速度信息		
			Last_Angle_Balance=Adc; //保存历史数据
			D_Count=0;	//计数器清零
		}
}

/**************************************************************************
函数功能：平滑 滤波
入口参数：速度
返回  值：滤波后的数据
**************************************************************************/
int Mean_Filter(int sensor)
{
  u8 i;
  s32 Sum_Speed = 0;     
	s16 Filter_Speed;   
  static  s16 Speed_Buf[FILTERING_TIMES]={0};
  for(i = 1 ; i<FILTERING_TIMES; i++)
  {
    Speed_Buf[i - 1] = Speed_Buf[i];
  }
  Speed_Buf[FILTERING_TIMES - 1] =sensor;

  for(i = 0 ; i < FILTERING_TIMES; i++)
  {
    Sum_Speed += Speed_Buf[i];
  }
  Filter_Speed = (s16)(Sum_Speed / FILTERING_TIMES);//
	return Filter_Speed;
}

/**************************************************************************
函数功能：位置式PID控制器
入口参数：编码器测量位置信息，目标位置
返回  值：电机PWM
根据位置式离散PID公式 
pwm=Kp*e(k)+Ki*∑e(k)+Kd[e（k）-e(k-1)]
e(k)代表本次偏差 
e(k-1)代表上一次的偏差  
∑e(k)代表e(k)以及之前的偏差的累积和;其中k为1,2,,k;
pwm代表输出
**************************************************************************/
int Position_PID (int Encoder,int Target)
{ 	
	
	  Position_Least =Encoder-Target;             //===
    Position_Bias *=0.8;		   
    Position_Bias += Position_Least*0.2;	             //===一阶低通滤波器  
	  Position_Differential=Position_Bias-Last_Position;
	  Last_Position=Position_Bias;
		Position_PWM=Position_Bias*Position_KP+Position_Differential*Position_KD; //===速度控制		
	  return Position_PWM;
}

/**************************************************************************
函数功能：顺摆位置PD控制 
入口参数：编码器
返回  值：位置控制PWM
作    者：平衡小车之家
**************************************************************************/
int Pre_Position(int Encoder)
{  
    static float Position_PWM,Last_Position,Position_Bias,Position_Differential;
  	Position_Bias =Encoder-Position_Zero; //===得到偏差
		Position_Differential=Position_Bias-Last_Position;//偏差积分
		Last_Position=Position_Bias;//保存上一次偏差
    Position_PWM=Position_Bias*25+Position_Differential*600; //===位置控制	
		return Position_PWM;//返回值
}

