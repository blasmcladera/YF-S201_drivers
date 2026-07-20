/*
 * Testing.c
 *
 * Author : Blas
 */ 

#include "yfs201.h"
#include "uart.h"
#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdio.h>

volatile uint8_t flag_uart_txlleno = 0;
char buffer_tx[BUFFER_SIZE];

int main(void)
{
	UART0_init();
    yfs201_init();
	sei();
	char message[64];
    while (1) 
    {
		_delay_ms(10);
		volatile float flow = yfs201_getFlow();
		uint16_t entero = (uint16_t)flow;
		uint16_t decimal = (uint16_t)((flow - entero) * 100);
		sprintf(
		message,
		"Actual Flow: %u.%02u  L/min\r\n",
		entero, decimal
		);
		while(flag_uart_txlleno);
		UART0_sendString(message);
    }
}

