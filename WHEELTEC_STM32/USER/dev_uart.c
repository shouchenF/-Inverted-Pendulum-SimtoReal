#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "bsp_uart.h"  // ����bsp_uart.c
#include "fifo.h"      // ����fifo.c
#include "dev_uart.h"

/* �����豸���ݽṹ */
typedef struct
{
	uint8_t status;		/* ����״̬ */
	_fifo_t tx_fifo;	/* ����fifo */
	_fifo_t rx_fifo;	/* ����fifo */
	uint8_t *dmarx_buf;	/* dma���ջ��� */
	uint16_t dmarx_buf_size;/* dma���ջ����С*/
	uint8_t *dmatx_buf;	/* dma���ͻ��� */
	uint16_t dmatx_buf_size;/* dma���ͻ����С */
	uint16_t last_dmarx_size;/* dma��һ�ν������ݴ�С */
}uart_device_t;

/* ���ڻ����С */
#define UART1_TX_BUF_SIZE           256
#define UART1_RX_BUF_SIZE           128
#define	UART1_DMA_RX_BUF_SIZE		128
#define	UART1_DMA_TX_BUF_SIZE		64

#define UART2_TX_BUF_SIZE           2048
#define UART2_RX_BUF_SIZE           2048
#define	UART2_DMA_RX_BUF_SIZE		256
#define	UART2_DMA_TX_BUF_SIZE		128

/* ���ڻ��� */
static uint8_t s_uart1_tx_buf[UART1_TX_BUF_SIZE];
static uint8_t s_uart1_rx_buf[UART1_RX_BUF_SIZE];
static uint8_t s_uart1_dmarx_buf[UART1_DMA_RX_BUF_SIZE];
static uint8_t s_uart1_dmatx_buf[UART1_DMA_TX_BUF_SIZE];

static uint8_t s_uart2_tx_buf[UART2_TX_BUF_SIZE];
static uint8_t s_uart2_rx_buf[UART2_RX_BUF_SIZE];
static uint8_t s_uart2_dmarx_buf[UART2_DMA_RX_BUF_SIZE];
static uint8_t s_uart2_dmatx_buf[UART2_DMA_TX_BUF_SIZE];

/* �����豸ʵ�� */
static uart_device_t s_uart_dev[2] = {0};

/* ���� */
uint32_t s_UartTxRxCount[4] = {0};


/* fifo�������� */
static void fifo_lock(void)
{
    __disable_irq();
}

/* fifo�������� */
static void fifo_unlock(void)
{
    __enable_irq();
}

/**
 * @brief �����豸��ʼ�� 
 * @param  
 * @retval 
 */
void uart_device_init(uint8_t uart_id)
{
  	if (uart_id == 0)
	{
		/* ���ô���1�շ�fifo */
		fifo_register(&s_uart_dev[uart_id].tx_fifo, &s_uart1_tx_buf[0], 
                      sizeof(s_uart1_tx_buf), fifo_lock, fifo_unlock);
		fifo_register(&s_uart_dev[uart_id].rx_fifo, &s_uart1_rx_buf[0], 
                      sizeof(s_uart1_rx_buf), fifo_lock, fifo_unlock);
		
		/* ���ô���1 DMA�շ�buf */
		s_uart_dev[uart_id].dmarx_buf = &s_uart1_dmarx_buf[0];
		s_uart_dev[uart_id].dmarx_buf_size = sizeof(s_uart1_dmarx_buf);
		s_uart_dev[uart_id].dmatx_buf = &s_uart1_dmatx_buf[0];
		s_uart_dev[uart_id].dmatx_buf_size = sizeof(s_uart1_dmatx_buf);
		bsp_uart1_dmarx_config(s_uart_dev[uart_id].dmarx_buf, 
							   sizeof(s_uart1_dmarx_buf));/* ֻ�����ý���ģʽDMA������ģʽ�跢������ʱ������ */
		s_uart_dev[uart_id].status  = 0;
	}
	else if (uart_id == 1)
	{
		/* ���ô���2�շ�fifo */
		fifo_register(&s_uart_dev[uart_id].tx_fifo, &s_uart2_tx_buf[0], 
                      sizeof(s_uart2_tx_buf), fifo_lock, fifo_unlock);
		fifo_register(&s_uart_dev[uart_id].rx_fifo, &s_uart2_rx_buf[0], 
                      sizeof(s_uart2_rx_buf), fifo_lock, fifo_unlock);
		
		/* ���ô���2 DMA�շ�buf */
		s_uart_dev[uart_id].dmarx_buf = &s_uart2_dmarx_buf[0];
		s_uart_dev[uart_id].dmarx_buf_size = sizeof(s_uart2_dmarx_buf);
		s_uart_dev[uart_id].dmatx_buf = &s_uart2_dmatx_buf[0];
		s_uart_dev[uart_id].dmatx_buf_size = sizeof(s_uart2_dmatx_buf);
		bsp_uart2_dmarx_config(s_uart_dev[uart_id].dmarx_buf, 
							   sizeof(s_uart2_dmarx_buf));
		s_uart_dev[uart_id].status  = 0;
	}
}

/**
 * @brief  ���ڷ������ݽӿڣ�ʵ����д�뷢��fifo��������dma����
 * @param  
 * @retval 
 */
uint16_t uart_write(uint8_t uart_id, const uint8_t *buf, uint16_t size)
{
	return fifo_write(&s_uart_dev[uart_id].tx_fifo, buf, size);
}

/**
 * @brief  ���ڶ�ȡ���ݽӿڣ�ʵ���Ǵӽ���fifo��ȡ
 * @param  
 * @retval 
 */
uint16_t uart_read(uint8_t uart_id, uint8_t *buf, uint16_t size)
{
	return fifo_read(&s_uart_dev[uart_id].rx_fifo, buf, size);
}

/**
 * @brief  ����dma��������жϴ���
 * @param  
 * @retval 
 */
void uart_dmarx_done_isr(uint8_t uart_id)
{
  	uint16_t recv_size;
	
	recv_size = s_uart_dev[uart_id].dmarx_buf_size - s_uart_dev[uart_id].last_dmarx_size;
    s_UartTxRxCount[uart_id*2+1] += recv_size;
	fifo_write(&s_uart_dev[uart_id].rx_fifo, 
				   (const uint8_t *)&(s_uart_dev[uart_id].dmarx_buf[s_uart_dev[uart_id].last_dmarx_size]), recv_size);

	s_uart_dev[uart_id].last_dmarx_size = 0;
}

/**
 * @brief  ����dma���ջ����Сһ�������жϴ���
 * @param  
 * @retval 
 */
void uart_dmarx_half_done_isr(uint8_t uart_id)
{
  	uint16_t recv_total_size;
  	uint16_t recv_size;
	
	if(uart_id == 0)
	{
	  	recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - bsp_uart1_get_dmarx_buf_remain_size();
	}
	else if (uart_id == 1)
	{
		recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - bsp_uart2_get_dmarx_buf_remain_size();
	}
	recv_size = recv_total_size - s_uart_dev[uart_id].last_dmarx_size;
	s_UartTxRxCount[uart_id*2+1] += recv_size;
    
	fifo_write(&s_uart_dev[uart_id].rx_fifo, 
				   (const uint8_t *)&(s_uart_dev[uart_id].dmarx_buf[s_uart_dev[uart_id].last_dmarx_size]), recv_size);
	s_uart_dev[uart_id].last_dmarx_size = recv_total_size;
}

/**
 * @brief  ���ڿ����жϴ���
 * @param  
 * @retval 
 */
void uart_dmarx_idle_isr(uint8_t uart_id)
{
  	uint16_t recv_total_size;
  	uint16_t recv_size;
	
	if(uart_id == 0)
	{
	  	recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - bsp_uart1_get_dmarx_buf_remain_size();
	}
	else if (uart_id == 1)
	{
		recv_total_size = s_uart_dev[uart_id].dmarx_buf_size - bsp_uart2_get_dmarx_buf_remain_size();
	}
	recv_size = recv_total_size - s_uart_dev[uart_id].last_dmarx_size;
	s_UartTxRxCount[uart_id*2+1] += recv_size;
	fifo_write(&s_uart_dev[uart_id].rx_fifo, 
				   (const uint8_t *)&(s_uart_dev[uart_id].dmarx_buf[s_uart_dev[uart_id].last_dmarx_size]), recv_size);
	s_uart_dev[uart_id].last_dmarx_size = recv_total_size;
}

/**
 * @brief  ����dma��������жϴ���
 * @param  
 * @retval 
 */
void uart_dmatx_done_isr(uint8_t uart_id)
{
 	s_uart_dev[uart_id].status = 0;	/* DMA���Ϳ��� */
}

/**
 * @brief  c
 * @param  
 * @retval 
 */
void uart_poll_dma_tx(uint8_t uart_id)
{
  	uint16_t size = 0;
	
	if (0x01 == s_uart_dev[uart_id].status)
    {
        return;
    }
	size = fifo_read(&s_uart_dev[uart_id].tx_fifo, s_uart_dev[uart_id].dmatx_buf,
					 s_uart_dev[uart_id].dmatx_buf_size);
	if (size != 0)
	{	
        s_UartTxRxCount[uart_id*2+0] += size;
	  	if (uart_id == 0)
		{
            s_uart_dev[uart_id].status = 0x01;	/* DMA����״̬ */
		  	bsp_uart1_dmatx_config(s_uart_dev[uart_id].dmatx_buf, size);
		}
		else if (uart_id == 1)
		{
            s_uart_dev[uart_id].status = 0x01;	/* DMA����״̬,������ʹ��DMA����ǰ��λ�������п���DMA�Ѿ����䲢�����ж� */
			bsp_uart2_dmatx_config(s_uart_dev[uart_id].dmatx_buf, size);
		}
	}
}

