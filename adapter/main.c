/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

#include <avr/io.h>

#include "genesis.h"
#include "genesis2.h"
#include "uart_helper.h"

#define F_CPU 8000000UL
#include <util/delay.h>

void
main (void)
{
	uint16_t state, ostate, state2, ostate2;

	setup_uart();

	genesis_setup();
	genesis2_setup();

	ostate = genesis_get_state();
	ostate2 = genesis2_get_state();

	while(1)
	{
		state = genesis_get_state();
		state2 = genesis2_get_state();

		state_comp(state, ostate, 0);
		state_comp(state2, ostate2, 1);

		ostate = state;
		ostate2 = state2;

		_delay_ms(2);

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
				genesis_setup();
				genesis2_setup();
			}
		}
	}
}
