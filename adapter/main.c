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

	// ------------- Defining pins:

	con1.input_ddr = &DDRC;
	con1.input_pin = &PINC;
	con1.input_port = &PORTC;
	con1.pins_d[0] = PC0;
	con1.pins_d[1] = PC1;
	con1.pins_d[2] = PC2;
	con1.pins_d[3] = PC3;
	con1.pins_d[4] = PC4;
	con1.pins_d[5] = PC5;

	con1.sel_ddr = &DDRB;
	con1.sel_port = &PORTB;
	con1.pin_sel = PB6;

	con2.input_ddr = &DDRD;
	con2.input_pin = &PIND;
	con2.input_port = &PORTD;
	con2.pins_d[0] = PD2;
	con2.pins_d[1] = PD3;
	con2.pins_d[2] = PD4;
	con2.pins_d[3] = PD5;
	con2.pins_d[4] = PD6;
	con2.pins_d[5] = PD7;
	
	con2.sel_ddr = &DDRB;
	con2.sel_port = &PORTB;
	con2.pin_sel = PB7;

	// ------------- Done

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
