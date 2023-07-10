#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
#define  DEBUG_USARTx                   USART1	

extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
//u8 Res;
//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);
#endif


