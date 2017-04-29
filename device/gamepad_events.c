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

const int transl_array[][2] =
{
	{(int) 'A', BTN_DPAD_UP},
	{(int) 'B', BTN_DPAD_DOWN},
	{(int) 'C', BTN_DPAD_LEFT},
	{(int) 'D', BTN_DPAD_RIGHT},
	{(int) 'E', BTN_A},
	{(int) 'F', BTN_B},
	{(int) 'G', BTN_C},
	{(int) 'H', BTN_Z},
	{(int) 'I', BTN_Y},
	{(int) 'J', BTN_X},
	{(int) 'K', BTN_MODE},
	{(int) 'L', BTN_START},
	{0,0},
};

int
get_event(struct input_event *ev, unsigned char action_byte)
{
	memset(ev, 0, sizeof(struct input_event));

	(*ev).type = EV_KEY;
	(*ev).value = action_byte >= 'a'? 1 : 0;

	if(action_byte >= 'a')
		action_byte -= 'a' - 'A';

	int done_flag = 0;

	for(int i = 0; transl_array[i][0] != 0; i++)
	{
		if((int) action_byte == transl_array[i][0])
		{
			(*ev).code = transl_array[i][1];
			done_flag = 1;
			break;
		}
	}

	if(done_flag == 0)
		return 1;
	else
		return 0;
}
