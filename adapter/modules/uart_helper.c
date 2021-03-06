/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/
#include <avr/io.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#define BAUD 38400
#include <util/setbaud.h>

#include "uart_helper.h"

void
setup_uart(void)
{
	UBRRH = UBRRH_VALUE; // Sets UART baudrate value detemined by setbaud.h
	UBRRL = UBRRL_VALUE;
	#if USE_2X
	UCSRA |= (1 << U2X); // Use 2x prescaler if needed
	#else
	UCSRA &= ~(1 << U2X);
	#endif
	UCSRB |= _BV(TXEN); // Enables UART TX and RX
	UCSRB |= _BV(RXEN);
}

void
flush_UART_RX_buffer(void)
{
	while(bit_is_set(UCSRA, RXC))
	{
		char foo = UDR;
	}
}

unsigned char
read_nb_char(void)
{
	if(bit_is_set(UCSRA, RXC))
	{
		return UDR;
	}

	return 0;
}

unsigned char
read_b_char(void)
{
	loop_until_bit_is_set(UCSRA, RXC);
	return UDR;
}

void
uart_print(const char *string) // Simple function for printing a string
{
	for (; *string; string++)
	{
		loop_until_bit_is_set(UCSRA, UDRE);
		UDR = *string;
	}
}

void
uart_print_bin(uint8_t num) // Simple function for printing a 8-bit number in decimal base
{
	for(uint8_t i = 0; i < 8; i++)
	{
		loop_until_bit_is_set(UCSRA, UDRE);
		UDR = '0' + ((num >> (7 - i)) % 2);
	}
}

void
uart_print_long_bin(uint16_t num) // Simple function for printing a 8-bit number in decimal base
{
	for(uint8_t i = 0; i < 16; i++)
	{
		loop_until_bit_is_set(UCSRA, UDRE);
		UDR = '0' + ((num >> (15 - i)) % 2);
	}
}
