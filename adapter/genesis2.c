/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

// Controller pin definitions:

#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>

#include "genesis2.h"
#include "uart_helper.h"

#define genesis2_port PORTD
#define genesis2_pin PIND
#define genesis2_ddr DDRD
#define genesis2_input_mask 0xFC
#define genesis2_input_shift 2

#define genesis2_sel_port PORTB
#define genesis2_sel_pin PINB
#define genesis2_sel_ddr DDRB
#define genesis2_sel_pin_n PB7

#define cdelay() _delay_us(20)

uint8_t six_buttons2 = 2;

void
genesis2_pulse_sel(uint8_t times)
{
	for(int i = 0; i < times; i++)
	{
		genesis2_sel_port &= ~_BV(genesis2_sel_pin_n);
		cdelay();
		genesis2_sel_port |= _BV(genesis2_sel_pin_n);
		cdelay();
	}
}

uint8_t
genesis2_get_data(uint8_t sel)
{
	if(sel == 0) // C, B, Right, Left, Down Up
	{
		uint8_t data = (genesis2_pin & genesis2_input_mask) >> genesis2_input_shift;

		return data;
	}
	else if(sel == 1) // Start, A, 0, 0, Down, Up
	{
		genesis2_sel_port &= ~_BV(genesis2_sel_pin_n);
		cdelay();
		uint8_t data = (genesis2_pin & genesis2_input_mask) >> genesis2_input_shift;
		genesis2_sel_port |= _BV(genesis2_sel_pin_n);
		cdelay();

		return data;
	}
	else if(sel == 2) // C, B, Mode, X, Y, Z
	{
		genesis2_pulse_sel(3);

		uint8_t data = (genesis2_pin & genesis2_input_mask) >> genesis2_input_shift;

		genesis2_pulse_sel(1);

		return data;
	}
}

void
genesis2_setup(void)
{
	uint8_t data_sel, data_tri, count = 0;

	genesis2_ddr &= ~(genesis2_input_mask); // Set pins connected to D0-D5 pins as input
	genesis2_port |= genesis2_input_mask; // and with pull-ups (is required?)

	genesis2_sel_ddr |= _BV(genesis2_sel_pin_n); // Sets pin connected to sel as output
	genesis2_sel_port |= _BV(genesis2_sel_pin_n); // and high (logic 1)

	// Detecting if it's a 3 or 6 buttons gamepad:
	// Don't press any buttons! (specially MODE and X)

	_delay_ms(50); // Very important delay!

	do
	{
		genesis2_pulse_sel(2);

		data_sel = genesis2_get_data(0);
		data_tri = genesis2_get_data(1);

		genesis2_pulse_sel(1);
	} while ( ( (data_sel & 0x0F) != 0x0F) && (count++ < 50) );

	if(count >= 50)
	{
		return;
	}

	uart_print(":1");

	if((data_tri & 0x0F) == 0x00)
	{
		six_buttons2 = 1;
	}
	else
	{
		six_buttons2 = 0;
	}

	_delay_ms(50); // Very important delay!
}

/* Buttons:

Up
Down
Left
Right
A
B
C
X
Y
Z
Start
Mode

uint8_t data1 = 0b00[C][B][R][L][D][U]
uint8_t data2 = 0b00[S][A] 0  0 [D][U]
uint8_t data3 = 0b00[C][B][M][X][Y][Z]

uint16_t state = 0b0000[S][M][X][Y][Z][C][B][A][R][L][D][U]

// [S][M][X][Y][Z][C][B][A][R][L][D][U]
//                         [R][L][D][U] (data1 % 0x0F)
//                      [A]             (data2 % 0x10)
//                [C][B]                (data1 % 0x30) << 1
//    [M][X][Y][Z]                      (data3 % 0x0F) << 7
// [S]                                  (data2 & 0x20) << 6

*/

uint16_t
genesis2_get_state()
{
	uint8_t data1, data2, data3 = 0;
	uint16_t state;

	if(six_buttons2 == 2)
	{
		return 0;
	}

	if(six_buttons2 == 1) // The order is important!
		data3 = ~genesis2_get_data(2);
	data2 = ~genesis2_get_data(1);
	data1 = ~genesis2_get_data(0);

	state = (data1 & 0x0F) | (data2 & 0x10) | (data1 & 0x30) << 1 | (data3 & 0x0F) << 7 | (data2 & 0x20) << 6;

	return state;
}