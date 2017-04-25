/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

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

#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>

#include "genesis.h"
#include "uart_helper.h"

#define cdelay() _delay_us(20)

static uint8_t dev_count;

void
genesis_reset()
{
	dev_count = 0;
}

void
genesis_pulse_sel(struct genesis_controller con, uint8_t times)
{
	for(int i = 0; i < times; i++)
	{
		*con.sel_port &= ~_BV(con.pin_sel);
		cdelay();
		*con.sel_port |= _BV(con.pin_sel);
		cdelay();
	}
}

uint8_t
genesis_get_data(struct genesis_controller con, uint8_t sel)
{
	uint8_t data = 0;

	if(sel == 0) // C, B, Right, Left, Down Up
	{
		for(uint8_t i = 0; i < 6; i++)
			data |= bit_is_set(*con.input_pin, con.pins_d[i])? 0 : _BV(i);

		return data;
	}
	else if(sel == 1) // Start, A, 0, 0, Down, Up
	{
		*con.sel_port &= ~_BV(con.pin_sel);
		cdelay();

		for(uint8_t i = 0; i < 6; i++)
			data |= bit_is_set(*con.input_pin, con.pins_d[i])? 0 : _BV(i);

		*con.sel_port |= _BV(con.pin_sel);
		cdelay();

		return data;
	}
	else if(sel == 2) // C, B, Mode, X, Y, Z
	{
		genesis_pulse_sel(con, 3);

		for(uint8_t i = 0; i < 6; i++)
			data |= bit_is_set(*con.input_pin, con.pins_d[i])? 0 : _BV(i);

		genesis_pulse_sel(con, 1);

		return data;
	}
}

void
genesis_setup(struct genesis_controller *con)
{
	uint8_t data_sel, data_tri, count = 0;

	*(*con).input_ddr &= ~(_BV((*con).pins_d[0]) |
	                      _BV((*con).pins_d[1]) |
	                      _BV((*con).pins_d[2]) |
	                      _BV((*con).pins_d[3]) |
	                      _BV((*con).pins_d[4]) |
	                      _BV((*con).pins_d[5])); // Set pins connected to D0-D5 pins as input

	*(*con).input_port |= (_BV((*con).pins_d[0]) |
	                      _BV((*con).pins_d[1]) |
	                      _BV((*con).pins_d[2]) |
	                      _BV((*con).pins_d[3]) |
	                      _BV((*con).pins_d[4]) |
	                      _BV((*con).pins_d[5])); // And with pullups

	*(*con).sel_ddr |= _BV((*con).pin_sel); // Sets pin connected to sel as output
	*(*con).sel_port |= _BV((*con).pin_sel); // and high (logic 1)

	// Detecting if it's a 3 or 6 buttons gamepad:
	// Don't press any buttons! (specially MODE and X)

	_delay_ms(50); // Very important delay!

	// TESTING GENESIS CONTROLLER PROTOCOL:
	// unsigned char testdata[10];

	// testdata[0] = genesis_get_data(*con, 0);
	// *(*con).sel_port &= ~_BV((*con).pin_sel);
	// cdelay();
	// testdata[1] = genesis_get_data(*con, 0);
	// *(*con).sel_port |= _BV((*con).pin_sel);
	// cdelay();
	// testdata[2] = genesis_get_data(*con, 0);
	// *(*con).sel_port &= ~_BV((*con).pin_sel);
	// cdelay();
	// testdata[3] = genesis_get_data(*con, 0);
	// *(*con).sel_port |= _BV((*con).pin_sel);
	// cdelay();
	// testdata[4] = genesis_get_data(*con, 0);
	// *(*con).sel_port &= ~_BV((*con).pin_sel);
	// cdelay();
	// testdata[5] = genesis_get_data(*con, 0);
	// *(*con).sel_port |= _BV((*con).pin_sel);
	// cdelay();
	// testdata[6] = genesis_get_data(*con, 0);
	// *(*con).sel_port &= ~_BV((*con).pin_sel);
	// cdelay();
	// testdata[7] = genesis_get_data(*con, 0);
	// *(*con).sel_port |= _BV((*con).pin_sel);
	// cdelay();
	// testdata[8] = genesis_get_data(*con, 0);
	// *(*con).sel_port &= ~_BV((*con).pin_sel);
	// cdelay();
	// testdata[9] = genesis_get_data(*con, 0);
	// *(*con).sel_port |= _BV((*con).pin_sel);
	// cdelay();

	// for(uint8_t i = 0; i < 10; i++)
	// {
	// 	uart_print("\r\ndata: ");
	// 	uart_print_bin(testdata[i]);
	// }

	do
	{
		genesis_pulse_sel(*con, 2);

		data_sel = genesis_get_data(*con, 0);
		data_tri = genesis_get_data(*con, 1);

		genesis_pulse_sel(*con, 1);
	} while ( ( (data_sel & 0x0F) != 0x00) && (count++ < 50) );

	if(count >= 50)
	{
		(*con).conf = 0;
		return;
	}

	(*con).dev_num = dev_count;

	char dev_init_str[] = "!\0";
	dev_init_str[1] = '0' + dev_count++;
	uart_print(dev_init_str);

	if((data_tri & 0x0F) == 0x0F)
	{
		(*con).conf = 3;
	}
	else
	{
		(*con).conf = 2;
	}

	_delay_ms(50);

	genesis_read_state(con);
	genesis_read_state(con);

	(*con).old_state = (*con).state;
}

void
genesis_read_state(struct genesis_controller *con)
{
	uint8_t data1, data2, data3 = 0;

	(*con).state = 0;

	if((*con).conf == 0)
	{
		return;
	}

	if((*con).conf == 3) // The order is important!
		data3 = genesis_get_data(*con, 2);
	data2 = genesis_get_data(*con, 1);
	data1 = genesis_get_data(*con, 0);

	(*con).state = (data1 & 0x0F) | (data2 & 0x10) | (data1 & 0x30) << 1 | (data3 & 0x0F) << 7 | (data2 & 0x20) << 6;
}

void
genesis_update_state(struct genesis_controller *con)
{
	for(uint8_t i = 0; i <= 12; i++)
	{
		if((*con).state != (*con).old_state)
		{
			if(((*con).state & _BV(i)) != ((*con).old_state & _BV(i)))
			{
				if(((*con).state & _BV(i)) != 0)
				{
					loop_until_bit_is_set(UCSRA, UDRE);
					UDR = '0' + (unsigned char) (*con).dev_num;
					loop_until_bit_is_set(UCSRA, UDRE);
					UDR = 'a' + (unsigned char) i;
				}
				else
				{
					loop_until_bit_is_set(UCSRA, UDRE);
					UDR = '0' + (unsigned char) (*con).dev_num;
					loop_until_bit_is_set(UCSRA, UDRE);
					UDR = 'A' + (unsigned char) i;
				}
			}
		}
	}

	(*con).old_state = (*con).state;
}
