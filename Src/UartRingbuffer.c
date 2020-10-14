/*
 * UartRingbuffer.c
 *
 *  Created on: 10-Jul-2019
 *      Author: ctltech
 *
 *  Modified on: 11-April-2020
 *  Modified on: 10-June-2020
 */

#include "UartRingbuffer.h"
#include "usart.h"
#include <string.h>

/**** define the UART you are using  ****/
/* put the following in the ISR 

extern void Uart_isr (UART_HandleTypeDef *huart);

*/

/****************=======================>>>>>>>>>>> NO CHANGES AFTER THIS =======================>>>>>>>>>>>**********************/

ring_buffer rx_buffer[4];
ring_buffer tx_buffer[4];

ring_buffer *_rx_buffer[4];
ring_buffer *_tx_buffer[4];

void store_char(unsigned char c, ring_buffer *buffer);

void Ringbuf_init(void)
{
	for (int i=0; i<END_OF_ALL_PORT_NO; i++)
	{
		_rx_buffer[i] = &rx_buffer[i];
		_tx_buffer[i] = &tx_buffer[i];
	}

  memset(&rx_buffer[0], 0x00, sizeof(rx_buffer));
  memset(&tx_buffer[0], 0x00, sizeof(tx_buffer));

  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_ENABLE_IT(DEBUG_PORT_H, UART_IT_ERR);

  /* Enable the UART Data Register not empty Interrupt */
  __HAL_UART_ENABLE_IT(DEBUG_PORT_H, UART_IT_RXNE);

  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_ENABLE_IT(LORA_PORT_H, UART_IT_ERR);

  /* Enable the UART Data Register not empty Interrupt */
  __HAL_UART_ENABLE_IT(LORA_PORT_H, UART_IT_RXNE);

  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_ENABLE_IT(BLE_PORT_H, UART_IT_ERR);

  /* Enable the UART Data Register not empty Interrupt */
  __HAL_UART_ENABLE_IT(BLE_PORT_H, UART_IT_RXNE);

  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_ENABLE_IT(SY7T609_PORT_H, UART_IT_ERR);

  /* Enable the UART Data Register not empty Interrupt */
  __HAL_UART_ENABLE_IT(SY7T609_PORT_H, UART_IT_RXNE);
}

void store_char(unsigned char c, ring_buffer *buffer)
{
  int i = (unsigned int)(buffer->head + 1) % UART_BUFFER_SIZE;

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if(i != buffer->tail) {
    buffer->buffer[buffer->head] = c;
    buffer->head = i;
  }
}

int Look_for (char *str, char *buffertolookinto)
{
	int stringlength = strlen (str);
	int bufferlength = strlen (buffertolookinto);
	int so_far = 0;
	int indx = 0;
repeat:
	while (str[so_far] != buffertolookinto[indx]) indx++;
	if (str[so_far] == buffertolookinto[indx]){
	while (str[so_far] == buffertolookinto[indx])
	{
		so_far++;
		indx++;
	}
	}

	else
		{
			so_far =0;
			if (indx >= bufferlength) return -1;
			goto repeat;
		}

	if (so_far == stringlength) return 1;
	else return -1;
}

int Uart_read(UartIndex port)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if(_rx_buffer[port]->head == _rx_buffer[port]->tail)
  {
	return -1;
  }
  else
  {
	unsigned char c = _rx_buffer[port]->buffer[_rx_buffer[port]->tail];
	_rx_buffer[port]->tail = (unsigned int)(_rx_buffer[port]->tail + 1) % UART_BUFFER_SIZE;
	return c;
  }
}

void Uart_write(UartIndex port, int c)
{
	UART_HandleTypeDef *p;

	if (c>=0)
	{
		int i = (_tx_buffer[port]->head + 1) % UART_BUFFER_SIZE;

		// If the output buffer is full, there's nothing for it other than to
		// wait for the interrupt handler to empty it a bit
		// ???: return 0 here instead?
		while (i == _tx_buffer[port]->tail);

		_tx_buffer[port]->buffer[_tx_buffer[port]->head] = (uint8_t)c;
		_tx_buffer[port]->head = i;

		switch (port)
		{
		case UART_DEBUG_NO:
			p = DEBUG_PORT_H;
			break;
		case LORA_PORT_NO:
			p = LORA_PORT_H;
			break;
		case BLE_PORT_NO:
			p = BLE_PORT_H;
			break;
		case SY7T609_PORT_NO:
			p = SY7T609_PORT_H;
			break;
		default:
			p = DEBUG_PORT_H;
			break;
		}

		__HAL_UART_ENABLE_IT(p, UART_IT_TXE); // Enable UART transmission interrupt
	}
}

int IsDataAvailable(UartIndex port)
{
  return (uint16_t)(UART_BUFFER_SIZE + _rx_buffer[port]->head - _rx_buffer[port]->tail) % UART_BUFFER_SIZE;
}

void Uart_isr (UART_HandleTypeDef *huart, UartIndex port)
{
	  uint32_t isrflags   = READ_REG(huart->Instance->SR);
	  uint32_t cr1its     = READ_REG(huart->Instance->CR1);

    /* if DR is not empty and the Rx Int is enabled */
    if (((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
    {
    	 /******************
    	    	      *  @note   PE (Parity error), FE (Framing error), NE (Noise error), ORE (Overrun
    	    	      *          error) and IDLE (Idle line detected) flags are cleared by software
    	    	      *          sequence: a read operation to USART_SR register followed by a read
    	    	      *          operation to USART_DR register.
    	    	      * @note   RXNE flag can be also cleared by a read to the USART_DR register.
    	    	      * @note   TC flag can be also cleared by software sequence: a read operation to
    	    	      *          USART_SR register followed by a write operation to USART_DR register.
    	    	      * @note   TXE flag is cleared only by a write to the USART_DR register.

    	 *********************/
		huart->Instance->SR;                       /* Read status register */
        unsigned char c = huart->Instance->DR;     /* Read data register */
        store_char (c, _rx_buffer[port]);  // store data in buffer
        return;
    }

    /*If interrupt is caused due to Transmit Data Register Empty */
    if (((isrflags & USART_SR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
    {
    	if(tx_buffer[port].head == tx_buffer[port].tail)
    	    {
    	      // Buffer empty, so disable interrupts
    	      __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);

    	    }

    	 else
    	    {
    	      // There is more data in the output buffer. Send the next byte
    	      unsigned char c = tx_buffer[port].buffer[tx_buffer[port].tail];
    	      tx_buffer[port].tail = (tx_buffer[port].tail + 1) % UART_BUFFER_SIZE;

    	      /******************
    	      *  @note   PE (Parity error), FE (Framing error), NE (Noise error), ORE (Overrun
    	      *          error) and IDLE (Idle line detected) flags are cleared by software
    	      *          sequence: a read operation to USART_SR register followed by a read
    	      *          operation to USART_DR register.
    	      * @note   RXNE flag can be also cleared by a read to the USART_DR register.
    	      * @note   TC flag can be also cleared by software sequence: a read operation to
    	      *          USART_SR register followed by a write operation to USART_DR register.
    	      * @note   TXE flag is cleared only by a write to the USART_DR register.

    	      *********************/

    	      huart->Instance->SR;
    	      huart->Instance->DR = c;

    	    }
    	return;
    }
}


// uart1 for debug
int console_getc()
{
	return Uart_read(UART_DEBUG_NO);
}

void console_putc(int c)
{
	Uart_write(UART_DEBUG_NO, c);
}

void console_puts(const char *s)
{
	while(*s) Uart_write(UART_DEBUG_NO, *s++);
}

int isgetc_from_console(void)
{
	if (IsDataAvailable(UART_DEBUG_NO))
		return 1;
	return 0;
}

// uart2 for lora
int lora_getc()
{
	return Uart_read(LORA_PORT_NO);
}

void lora_putc(int c)
{
	Uart_write(LORA_PORT_NO, c);
}

void lora_puts(const char *s)
{
	while(*s) Uart_write(LORA_PORT_NO, *s++);
}

int isgetc_from_lora(void)
{
	if (IsDataAvailable(LORA_PORT_NO))
		return 1;
	return 0;
}

// uart3 for ble
int ble_getc()
{
	return Uart_read(BLE_PORT_NO);
}

void ble_putc(int c)
{
	Uart_write(BLE_PORT_NO, c);
}

void ble_puts(const char *s)
{
	while(*s) Uart_write(BLE_PORT_NO, *s++);
}


int isgetc_from_ble(void)
{
	if (IsDataAvailable(BLE_PORT_NO))
		return 1;
	return 0;
}

// uart6 for sy7t609
int sy7t609_getc()
{
	return Uart_read(SY7T609_PORT_NO);
}

void sy7t609_putc(int c)
{
	Uart_write(SY7T609_PORT_NO, c);
}

void sy7t609_puts(const char *s)
{
	while(*s) Uart_write(SY7T609_PORT_NO, *s++);
}


int isgetc_from_sy7t609(void)
{
	if (IsDataAvailable(SY7T609_PORT_NO))
		return 1;
	return 0;
}
