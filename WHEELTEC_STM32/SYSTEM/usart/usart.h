#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define USART_REC_LEN  			7  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
#define  DEBUG_USARTx                   USART1	

// 串口对应的DMA请求通道
#define  USART_TX_DMA_CHANNEL     DMA1_Channel5
// 外设寄存器地址
#define  USART_DR_ADDRESS        (USART1_BASE+0x04)
// 一次发送的数据量
#define  RECEIVEBUFF_SIZE            5000


extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记

extern uint16_t Res;
extern float action;
//u8 Res;
//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch);
void Usart_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint16_t num);
static void usart1_dma_tx_init(void);
static void usart1_dma_rx_init(void);
uint8_t usart1_dma_tx_data(void* buffer, uint16_t data_lenth);
void my_usmart_scan(uint8_t * data_array,uint8_t data_lenth);
#endif


