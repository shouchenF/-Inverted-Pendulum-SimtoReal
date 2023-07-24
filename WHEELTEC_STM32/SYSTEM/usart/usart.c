#include "sys.h"
#include "usart.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
uint16_t Res;
float action;
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif  

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
////重定义fputc函数 
//int fputc(int ch, FILE *f)
//{      
//	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
//    USART1->DR = (u8) ch;      
//	return ch;
//}
#endif 

 
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
  //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}

// 奇偶校验
int16_t count_odd_numbers(int16_t a) // a:数据起始位，b:数据长度
{
    int16_t odd_count = 0;
    int16_t even_count = 0;
    for (int16_t i = a; i < (USART_RX_STA&0X3FFF); i++) {	
				if(USART_RX_BUF[i] >= '0' && USART_RX_BUF[i] <= '9' )
				{						
						if ((USART_RX_BUF[i] - '0') % 2 == 0) {
								even_count++;
						} else {
								odd_count++;
						}
				}   
	}
	return odd_count;		
}


void USART1_IRQHandler(void)                	//串口1中断服务程序
	{
	 uint16_t check_flag;

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
		Res = USART_ReceiveData(USART1);	//读取接收到的数据 0x2D，0x31，0x30，0x30，0x30，0x0D，0x0A

				if((USART_RX_STA&0x8000)==0)//接收未完成
					{
					if(USART_RX_STA&0x4000)//接收到了0x0D
						{
							if(Res!=0x0A){
								USART_RX_STA=0;//接收错误,重新开始
								memset(USART_RX_BUF,0,USART_REC_LEN);
							}
							else{
								USART_RX_STA|=0x8000;	//接收完成了
								
								check_flag = count_odd_numbers(2);
								USART_RX_BUF[1] = USART_RX_BUF[1] - '0';
								if(USART_RX_BUF[0] == '\t'&& USART_RX_BUF[1] == check_flag ) // 判断帧头是否正确、判断奇偶校验位是否正确 || USART_RX_BUF[1] == check_flag
								{
									float value = 0;
									int16_t sign = 1;
										int a = 2;
										if(USART_RX_BUF[2] == '-')
										{
											sign = -1;
											a = 3;
										}
											for (int i = a; i < (USART_RX_STA&0X3FFF); i++) {
												if(USART_RX_BUF[i] != '.')
												{
//												 value = value * 10 + USART_RX_BUF[i] - '0';	
												 value +=  (USART_RX_BUF[i] - '0') * pow(10, -(i - a));
												}
												else
												{
													a = a+ 1;
												}
												
											}
											action = sign * value;
											USART_RX_STA = 0;	
											memset(USART_RX_BUF,0,USART_REC_LEN);
							}
							else{
								USART_RX_STA = 0;	
								memset(USART_RX_BUF,0,USART_REC_LEN);
							}
						}								
						}
					else //还没收到0X0D
						{	
							if(Res==0x0D)
								USART_RX_STA|=0x4000;
							else
								{					
										USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
										USART_RX_STA++;
										if(USART_RX_STA>(USART_REC_LEN-1))
										{
											USART_RX_STA=0;//接收数据错误,重新开始接收
											memset(USART_RX_BUF,0,USART_REC_LEN);
										}					
								}		 
						}
					}

				 } 
}
	
/*****************  发送一个字符 **********************/
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* 发送一个字节数据到USART */
	USART_SendData(pUSARTx,ch);
		
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

/*****************  发送字符串 **********************/
void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
{
	unsigned int k=0;
  do 
  {
      Usart_SendByte( pUSARTx, *(str + k) );
      k++;
  } while(*(str + k)!='\0');
  
  /* 等待发送完成 */
  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET)
  {}
}

/*****************  发送一个16位数 **********************/
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch)
{
	uint8_t temp_h, temp_l;
	
	/* 取出高八位 */
	temp_h = (ch&0XFF00)>>8;
	/* 取出低八位 */
	temp_l = ch&0XFF;
	
	/* 发送高八位 */
	USART_SendData(pUSARTx,temp_h);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	
	/* 发送低八位 */
	USART_SendData(pUSARTx,temp_l);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

///重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到串口 */
		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
		/* 等待串口输入数据 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USARTx);
}


#endif	

