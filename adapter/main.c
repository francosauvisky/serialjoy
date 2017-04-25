/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

#include <avr/io.h>

#include "modules/genesis.h"
#include "modules/uart_helper.h"

#define F_CPU 8000000UL
#include <util/delay.h>

void
main (void)
{
	struct genesis_controller con1, con2;

	genesis_set_pinmap(&con1, &DDRC, &PINC, &PORTC,
		PC0, PC1, PC2, PC3, PC4, PC5, &DDRB, &PORTB, PB6);
 	genesis_set_pinmap(&con2, &DDRD, &PIND, &PORTD,
 		PD2, PD3, PD4, PD5, PD6, PD7, &DDRB, &PORTB, PB7);

	setup_uart();

	genesis_setup(&con1);
	genesis_setup(&con2);

	while(1)
	{
		genesis_read_state(&con1);
		genesis_read_state(&con2);

		genesis_update_state(&con1);
		genesis_update_state(&con2);

		_delay_ms(1);

		unsigned char recv = read_nb_char();
		if(recv != 0)
		{
			if(recv == '?')
			{
				loop_until_bit_is_set(UCSRA, UDRE);
				UDR = '#';
			}
			else if(recv == 'd')
			{
				genesis_reset();
				genesis_setup(&con1);
				genesis_setup(&con2);
			}
		}
	}
}
