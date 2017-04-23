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

int
get_event(struct input_event *ev, unsigned char action_byte)
{
	memset(ev, 0, sizeof(struct input_event));

	(*ev).type = EV_KEY;
	(*ev).value = action_byte >= 'a'? 1 : 0;

	if(action_byte >= 'a')
		action_byte -= 'a' - 'A';

	switch(action_byte)
	{
		case 'A':
			(*ev).code = BTN_DPAD_UP;
			break;
		case 'B':
			(*ev).code = BTN_DPAD_DOWN;
			break;
		case 'C':
			(*ev).code = BTN_DPAD_LEFT;
			break;
		case 'D':
			(*ev).code = BTN_DPAD_RIGHT;
			break;
		case 'E':
			(*ev).code = BTN_A;
			break;
		case 'F':
			(*ev).code = BTN_B;
			break;
		case 'G':
			(*ev).code = BTN_C;
			break;
		case 'H':
			(*ev).code = BTN_Z;
			break;
		case 'I':
			(*ev).code = BTN_Y;
			break;
		case 'J':
			(*ev).code = BTN_X;
			break;
		case 'K':
			(*ev).code = BTN_MODE;
			break;
		case 'L':
			(*ev).code = BTN_START;
			break;
		default:
			return -1;
	}

	printf("%c %d\n", action_byte, (*ev).value);

	return 0;
}
