/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

/*
The received byte has the following form:

A-Z or a-z:
The character indicates the button

a-A:
Uppercase -> Button pressed
Lowercase -> Button relased
*/

#include <stdio.h>  /* Standard input/output definitions */
#include <string.h> /* String function definitions */
#include <linux/input.h>

#include "defs.h"

const int key_transl_array[][2] =
{
	{(int) 'A', BTN_DPAD_UP},
	{(int) 'B', BTN_DPAD_DOWN},
	{(int) 'C', BTN_DPAD_LEFT},
	{(int) 'D', BTN_DPAD_RIGHT},
	{(int) 'E', BTN_Y},
	{(int) 'F', BTN_B},
	{(int) 'G', BTN_A},
	{(int) 'H', BTN_TR},
	{(int) 'I', BTN_X},
	{(int) 'J', BTN_TL},
	{(int) 'K', BTN_MODE},
	{(int) 'L', BTN_START},
	{(int) 'M', BTN_TR2},
	{(int) 'N', BTN_TL2},
	{0,0},
};

const int abs_transl_array[][2] =
{
	{(int) 'A', ABS_X},
	{(int) 'B', ABS_Y},
	{(int) 'C', ABS_RX},
	{(int) 'D', ABS_RY},
	{0,0},
};

int
get_key_event(struct input_event *ev, struct data_packet pkg)
{
	memset(ev, 0, sizeof(struct input_event));

	ev->type = EV_KEY;
	ev->value = pkg.a_data >= 'a'? 1 : 0;

	if(pkg.a_data >= 'a')
		pkg.a_data -= 'a' - 'A';

	int done_flag = 0;

	for(int i = 0; key_transl_array[i][0] != 0; i++)
	{
		if((int) pkg.a_data == key_transl_array[i][0])
		{
			ev->code = key_transl_array[i][1];
			done_flag = 1;
			break;
		}
	}

	if(done_flag == 0)
		return 1;
	else
		return 0;
}

int
get_abs_event(struct input_event *ev, struct data_packet pkg)
{
	memset(ev, 0, sizeof(struct input_event));

	ev->type = EV_ABS;
	ev->value = (int) ((pkg.l_data & 0x0F) + ((pkg.h_data & 0x0F) << 4));

	int done_flag = 0;

	for(int i = 0; abs_transl_array[i][0] != 0; i++)
	{
		if((int) pkg.a_data == abs_transl_array[i][0])
		{
			ev->code = abs_transl_array[i][1];
			done_flag = 1;
			break;
		}
	}

	if(done_flag == 0)
		return 1;
	else
		return 0;
}
