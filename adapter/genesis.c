/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

// Controller pin definitions:

#define genesis_port PORTC
#define genesis_pin PINC
#define genesis_ddr DDRC
#define genesis_input_mask 0x3F
#define genesis_input_shift 0

#define genesis_sel_port PORTB
#define genesis_sel_pin PINB
#define genesis_sel_ddr DDRB
#define genesis_sel_pin_n PB6

#define cdelay() _delay_us(20)

uint8_t six_buttons;

uint8_t genesis_pulse_sel(uint8_t times)
{
	for(int i = 0; i < times; i++)
	{
		genesis_sel_port &= ~_BV(genesis_sel_pin_n);
		cdelay();
		genesis_sel_port |= _BV(genesis_sel_pin_n);
		cdelay();
	}
}

uint8_t
genesis_get_data(uint8_t sel)
{
	if(sel == 0) // C, B, Right, Left, Down Up
	{
		uint8_t data = (genesis_pin & genesis_input_mask) >> genesis_input_shift;

		return data;
	}
	else if(sel == 1) // Start, A, 0, 0, Down, Up
	{
		genesis_sel_port &= ~_BV(genesis_sel_pin_n);
		cdelay();
		uint8_t data = (genesis_pin & genesis_input_mask) >> genesis_input_shift;
		genesis_sel_port |= _BV(genesis_sel_pin_n);
		cdelay();

		return data;
	}
	else if(sel == 2) // C, B, Mode, X, Y, Z
	{
		genesis_pulse_sel(3);

		uint8_t data = (genesis_pin & genesis_input_mask) >> genesis_input_shift;

		genesis_pulse_sel(1);

		return data;
	}

	return 0;
}

uint16_t
genesis_setup(void)
{
	uint8_t data_sel, data_tri;

	genesis_ddr &= ~(genesis_input_mask); // Set pins connected to D0-D5 pins as input
	genesis_port |= genesis_input_mask; // and with pull-ups (is required?)

	genesis_sel_ddr |= _BV(genesis_sel_pin_n); // Sets pin connected to sel as output
	genesis_sel_port |= _BV(genesis_sel_pin_n); // and high (logic 1)

	// Detecting if it's a 3 or 6 buttons gamepad:
	// Don't press any buttons! (specially MODE and X)

	_delay_ms(50); // Very important delay!
	do
	{
		genesis_pulse_sel(2);

		data_sel = genesis_get_data(0);
		data_tri = genesis_get_data(1);

		genesis_pulse_sel(1);
	} while (data_sel & 0x0F != 0x0F);

	if((data_tri & 0x0F) == 0x00)
	{
		six_buttons = 1;
	}
	else
	{
		six_buttons = 0;
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
genesis_get_state()
{
	uint8_t data1, data2, data3 = 0;
	uint16_t state;

	if(six_buttons) // The order is important!
		data3 = ~genesis_get_data(2);
	data2 = ~genesis_get_data(1);
	data1 = ~genesis_get_data(0);

	state = (data1 & 0x0F) | (data2 & 0x10) | (data1 & 0x30) << 1 | (data3 & 0x0F) << 7 | (data2 & 0x20) << 6;

	return state;
}

void
state_comp(uint16_t state, uint16_t ostate)
{
	for(uint8_t i = 0; i <= 12; i++)
	{
		if(state != ostate)
		{
			if((state & _BV(i)) != (ostate & _BV(i)))
			{
				if((state & _BV(i)) != 0)
				{
					loop_until_bit_is_set(UCSRA, UDRE);
					UDR = 'a' + (unsigned char) i;
				}
				else
				{
					loop_until_bit_is_set(UCSRA, UDRE);
					UDR = 'A' + (unsigned char) i;
				}
			}
		}
	}
}
