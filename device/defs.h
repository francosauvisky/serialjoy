/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

#ifndef gamepad_events_header
#define gamepad_events_header

#include <linux/input.h>

//gamepad_events:
int get_event(struct input_event *, unsigned char);

//simple_serial:
int open_port(char *);
unsigned char read_char(int);
void print_char(int fd, unsigned char);

//simple_uinput:
int open_uinput(void);
void setup_uinput(int, char *);

#endif
