#include "sys.h"
#include "usart.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
uint16_t Res;
float action;
uint16_t check_flag;

#define USART1_RX_BUFF_MAX_LENTH 256    //定义最大接收字节数 200
#define USART1_TX_BUFF_MAX_LENTH 256

uint8_t ReceiveBuff[RECEIVEBUFF_SIZE];
static uint8_t usart1_tx_buff[USART1_TX_BUFF_MAX_LENTH];
uint8_t usart1_rx_buff[USART1_RX_BUFF_MAX_LENTH];

typedef struct
{
	uint8_t rxc;
	uint8_t txc;
}uart_state_t;

uart_state_t USART1_state;

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
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//使能USART1时钟
  RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1, ENABLE); //复位串口1
  RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1, DISABLE);
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIOA时钟
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //TX PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//RX PA.10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级1
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
	
//  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断 ???
  USART_ITConfig(USART1, USART_IT_IDLE, ENABLE); // 空闲中断,打开接收中断。
  USART_Cmd(USART1, ENABLE);                    //使能串口1 
	usart1_dma_tx_init();
	usart1_dma_rx_init();	
}


//static void usart1_dma_tx_init(void)
//{
//    DMA_InitTypeDef DMA_InitStructure;
//    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
//    DMA_DeInit(DMA1_Channel4);
//    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;
//    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)usart1_tx_buff;
//    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
//    DMA_InitStructure.DMA_BufferSize = 0;/*避免初始化发送数据，设置为0*/
//    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
//    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
//    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//    DMA_Init(DMA1_Channel4, &DMA_InitStructure);
//    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
//    DMA_Cmd(DMA1_Channel4, ENABLE);
//}

//uint8_t usart1_dma_tx_data(void* buffer, uint16_t data_lenth)
//{
//	if(data_lenth < 1)// 判断长度是否有效
//	{
//		return ERROR;
//	}
//	
//	while (DMA_GetCurrDataCounter(DMA1_Channel4));// 检查DMA发送通道内是否还有数据
//	if(NULL == buffer)//指针判空
//	{
//		return ERROR;
//	}
//	
//	USART1_state.txc = RESET;
//	
//	memcpy(usart1_tx_buff, buffer,((data_lenth > USART1_TX_BUFF_MAX_LENTH) ? USART1_TX_BUFF_MAX_LENTH:data_lenth));
//	
//    DMA_Cmd(DMA1_Channel4, DISABLE); //DMA发送数据-要先关 设置发送长度 开启DMA
//    DMA_SetCurrDataCounter(DMA1_Channel4, data_lenth);// 设置发送长度
//    DMA_Cmd(DMA1_Channel4, ENABLE);// 启动DMA发送
//	
//	return SUCCESS;
//}


//static void usart1_dma_rx_init(void)
//{
//    DMA_InitTypeDef DMA_InitStructure;
//    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
//    DMA_DeInit(DMA1_Channel5);
//    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;           // 初始化外设地址
//    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)usart1_rx_buff;            // 缓存地址
//    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                     // 外设作为数据来源
//    DMA_InitStructure.DMA_BufferSize = USART1_RX_BUFF_MAX_LENTH;           // 缓存容量
//    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;       // 外设地址不递增
//    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                // 内存递增
//    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;// 外设字节宽度
//    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;        // 内存字节宽度
//    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;						   // 正常模式，即满了就不在接收了，而不是循环存储
//    DMA_InitStructure.DMA_Priority = DMA_Priority_High;                    // 优先级很高
//    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                           // 内存与外设通信，而非内存到内存
//    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
//    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
//    DMA_Cmd(DMA1_Channel5, ENABLE);
//}

//void USART1_IRQHandler(void)
//{
//    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)/*空闲中断接收*/
//    {
//        u8 data_lenth = 0 ;
//		
//        DMA_Cmd(DMA1_Channel5, DISABLE);// 关闭DMA ，防止干扰
//				USART_ReceiveData( USART1 );
//		
//        data_lenth = USART1_RX_BUFF_MAX_LENTH - DMA_GetCurrDataCounter(DMA1_Channel5);//获得接收到的字节数
//        DMA_SetCurrDataCounter(DMA1_Channel5, USART1_RX_BUFF_MAX_LENTH);//  重新赋值计数值，必须大于等于最大可能接收到的数据帧数目
//        DMA_Cmd(DMA1_Channel5, ENABLE);
//				USART1_state.rxc = SET;  //设置接收完成标志
//		
//				my_usmart_scan(usart1_rx_buff,data_lenth); //配置自己的接收处理函数//执行usmart扫描	
//		
//        memset(usart1_rx_buff, 0, USART1_RX_BUFF_MAX_LENTH); //清空接收缓存区		 
//				USART_ClearITPendingBit(USART1, USART_IT_IDLE); // Clear IDLE interrupt flag bit
//    }
//		if(USART_GetITStatus(USART1,USART_IT_TC) != RESET)  //发送完成标记
//		{
//			DMA_Cmd(DMA1_Channel4, DISABLE);                // 关闭DMA
//			DMA_SetCurrDataCounter(DMA1_Channel4,RESET);    // 清除数据长度
//			USART1_state.txc = SET;                         // 设置发送完成标志
//			USART_ClearITPendingBit(USART1, USART_IT_TC);   // 清除完成标记
//		}	
//}
	 
///********************************************************
//Function:   void my_usmart_scan(uint8_t * data_array,uint8_t data_lenth)
//Description:处理接收数据
//Input:
//Output:
//Others:
//*********************************************************/
//void my_usmart_scan(uint8_t * data_array,uint8_t data_lenth)
//{
//	uint8_t sta,len;  
//	if(SET == USART1_state.rxc)//串口接收完成？
//	{					   
//		len = data_lenth;	//得到此次接收到的数据长度
//			if(data_array[0] == 0x2D && data_array[1] == 0x01 && data_array[5] == 0x56 && data_array[6] == 0x78) // 判断帧头是否正确、判断奇偶校验位是否正确 || USART_RX_BUF[1] == check_flag
//			{
//				float value = 0;
//				int16_t sign = 1;
//					if(data_array[2] == 0x45)
//					{
//						sign = -1;
//					}
//					value = (data_array[4] << 8) + data_array[3];	
//					action = sign * value;
//		}																			
//		USART1_state.rxc = RESET;//状态寄存器清空	    
//	}	
//}	 
//	 
	 
//// 奇偶校验
//int16_t count_odd_numbers(int16_t a) // a:数据起始位，b:数据长度
//{
//    int16_t odd_count = 0;
//    int16_t even_count = 0;
//    for (int16_t i = 3; i < (USART_RX_STA&0X3FFF); i++) {	
//			
//				if(USART_RX_BUF[i] >= '0' && USART_RX_BUF[i] <= '9' )
//				{						
//						if ((USART_RX_BUF[i] - '0') % 2 == 0) {
//								even_count++;
//						} else {
//								odd_count++;
//						}
//		  
//	}
//	return odd_count;		
//}

// 奇偶校验
//int16_t count_odd_numbers() // a:数据起始位，b:数据长度
//{
//    int16_t odd_count = 0;
//    int16_t even_count = 0;
//						
//		if (((USART_RX_BUF[4] << 8) + USART_RX_BUF[3]) % 2 == 0) {
//				even_count++;
//		} else {
//				odd_count++;
//		}
//	return odd_count;		
//}


//void USART1_IRQHandler(void)                	//串口1中断服务程序
//	{
//	

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
		Res = USART_ReceiveData(USART1);	//读取接收到的数据 0x2D，0x31，0x30，0x30，0x30，0x0D，0x0A
		printf("%02X", Res);
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
								
//								check_flag = count_odd_numbers();
//								USART_RX_BUF[1] = USART_RX_BUF[1] - '0';
								if(USART_RX_BUF[0] == 0x2D && USART_RX_BUF[1] == 0x01 ) // 判断帧头是否正确、判断奇偶校验位是否正确 || USART_RX_BUF[1] == check_flag
								{
									float value = 0;
									int16_t sign = 1;
										if(USART_RX_BUF[2] == 0x45)
										{
											sign = -1;
										}

										  value = (USART_RX_BUF[4] << 8) + USART_RX_BUF[3];	
			
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
	

//void USART1_IRQHandler(void)                	//串口1中断服务程序
//	{
//	

//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
//		{
//		Res = USART_ReceiveData(USART1);	//读取接收到的数据 0x2D，0x31，0x30，0x30，0x30，0x0D，0x0A

//				if((USART_RX_STA&0x8000)==0)//接收未完成
//					{
//					if(USART_RX_STA&0x4000)//接收到了0x0D
//						{
//							if(Res!=0x0A){
//								USART_RX_STA=0;//接收错误,重新开始
//								memset(USART_RX_BUF,0,USART_REC_LEN);
//							}
//							else{
//								USART_RX_STA|=0x8000;	//接收完成了
//								
//								check_flag = count_odd_numbers(2);
//								USART_RX_BUF[1] = USART_RX_BUF[1] - '0';
//								if(USART_RX_BUF[0] == '\t'&& USART_RX_BUF[1] == check_flag ) // 判断帧头是否正确、判断奇偶校验位是否正确 || USART_RX_BUF[1] == check_flag
//								{
//									float value = 0;
//									int16_t sign = 1;
//										int a = 2;
//										if(USART_RX_BUF[2] == '-')
//										{
//											sign = -1;
//											a = 3;
//										}
//											for (int i = a; i < (USART_RX_STA&0X3FFF); i++) {
////												if(USART_RX_BUF[i] != '.')
////												{
//												 value = value * 10 + USART_RX_BUF[i] - '0';	
////												 value +=  (USART_RX_BUF[i] - '0') * pow(10, -(i - a));
////												}
////												else
////												{
////													a = a+ 1;
////												}
////												
//											}
//											action = sign * value;
//											USART_RX_STA = 0;	
//											memset(USART_RX_BUF,0,USART_REC_LEN);
//							}
//							else{
//								USART_RX_STA = 0;	
//								memset(USART_RX_BUF,0,USART_REC_LEN);
//							}
//						}								
//						}
//					else //还没收到0X0D
//						{	
//							if(Res==0x0D)
//								USART_RX_STA|=0x4000;
//							else
//								{					
//										USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
//										USART_RX_STA++;
//										if(USART_RX_STA>(USART_REC_LEN-1))
//										{
//											USART_RX_STA=0;//接收数据错误,重新开始接收
//											memset(USART_RX_BUF,0,USART_REC_LEN);
//										}					
//								}		 
//						}
//					}

//				 } 
//}
//	

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

