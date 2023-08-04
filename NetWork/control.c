	/**************************************************************************
函数功能：倾角PD控制
入口参数：角度
返回  值：倾角控制PWM
**************************************************************************/
 int Balance(float Angle)
 {
 	 Bias=Angle-ZHONGZHI;              //求出平衡的角度中值 和机械相关
 	 D_Bias=Bias-Last_Bias;            //求出偏差的微分 进行微分控制
 	 balance=-Balance_KP*Bias-D_Bias*Balance_KD;   //===计算倾角控制的电机PWM  PD控制
   Last_Bias=Bias;                   //保持上一次的偏差
 	 return balance;
 }

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


void control_motor(){
    Turn_Off(Voltage);//倾角、电压保护
    if(Swing_up==0) Position_Zero=Encoder,Last_Position=0,Last_Bias=0,Position_Target=0,Swing_up=1;

    if(Flag_Stop==0)
    {
        Balance_Pwm =Balance(Angle_Balance);                                          //===角度PD控制
        if(++Position_Target>4) Position_Pwm=Position(Encoder),Position_Target=0;    //===位置PD控制 25ms进行一次位置控制
        Moto=Balance_Pwm-Position_Pwm;      //===计算电机最终PWM
        Xianfu_Pwm();                         //===PWM限幅 防止占空比100%带来的系统不稳定因素

    }
}


