/*
 * UartRingbuffer.h
 *
 *  Created on: 10-Jul-2019
 *      Author: ctltech
 */

#ifndef UARTRINGBUFFER_H_
#define UARTRINGBUFFER_H_

#include "stm32f4xx_hal.h"
#include "main.h"

/* change the size of the buffer */
#define UART_BUFFER_SIZE 256		//64

typedef struct
{
  unsigned char buffer[UART_BUFFER_SIZE];
  volatile unsigned int head;
  volatile unsigned int tail;
} ring_buffer;


/* Initialize the ring buffer */
void Ringbuf_init(void);

/* the ISR for the uart. put it in the IRQ handler */
	void Uart_isr (UART_HandleTypeDef *huart, UartIndex port);

/* reads the data in the rx_buffer and increment the tail count in rx_buffer */
	int Uart_read(UartIndex port);

/* writes the data to the tx_buffer and increment the head count in tx_buffer */
	void Uart_write(UartIndex port, int c);

/* Checks if the data is available to read in the rx_buffer */
	int IsDataAvailable(UartIndex port);

/* reads the data from console */
	int console_getc(void);

/* send the data to console */
	void console_putc(int c);

/* checks if the data is available to read from ble module */
	int isgetc_from_ble(void);

/* checks if the data is available to read from console */
	int isgetc_from_console(void);

/* reads the data from ble module */
	int ble_getc(void);

/* send the data to ble module in char*/
	void ble_putc(int c);

/* send the data to ble module in string */
	void ble_puts(const char *s);

/* reads the data from lora module */
	int lora_getc();

/* send the data to lora module in char */
	void lora_putc(int c);

/* sedn the data to lora module in string */
	void lora_puts(const char *s);

/* checks if the data is available to read from lora */
	int isgetc_from_lora(void);

/* reads the data from sy7t609 */
	int sy7t609_getc();

/* send to sy7t609 in char */
	void sy7t609_putc(int c);

/* send to sy7t609 in string */
	void sy7t609_puts(const char *s);

/* checks if the data is available to read from sy7t609 */
	int isgetc_from_sy7t609(void);


#endif /* UARTRINGBUFFER_H_ */
