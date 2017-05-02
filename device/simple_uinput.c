/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

#include <stdio.h>  /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h> /* String function definitions */
#include <unistd.h> /* UNIX standard function definitions */
#include <fcntl.h>  /* File control definitions */

#include <linux/input.h>
#include <linux/uinput.h>

#include "defs.h"

void
open_uinput(struct uinput_controller *gamepad)
{
	gamepad->fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

	if(gamepad->fd < 0) // If it fails
	{
		gamepad->fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
	}
	if(gamepad->fd < 0)
		die("error: open/open_uinput");

	gamepad->status = 0;
}

void
setup_gamepad(struct uinput_controller *gamepad, char *dev_name)
{
	if(ioctl(gamepad->fd, UI_SET_EVBIT, EV_SYN) < 0) // Enables EV_SYN events
		die("error: ioctl/setup_uinput");

	if(ioctl(gamepad->fd, UI_SET_EVBIT, EV_KEY) < 0) // Enables EV_KEY events
		die("error: ioctl/setup_uinput");
	for(char i = 'A'; i <= 'Z'; i++)
	{
		struct input_event foo;
		struct data_packet bar;
		bar.a_data = i;
		get_key_event(&foo, bar);

		if(ioctl(gamepad->fd, UI_SET_KEYBIT, foo.code) < 0) // Enables every key required
			die("error: ioctl/setup_uinput");
	}

	if(ioctl(gamepad->fd, UI_SET_EVBIT, EV_ABS) < 0)
		die("error: ioctl/setup_uinput");
	for(char i = 'A'; i <= 'Z'; i++)
	{
		struct input_event foo;
		struct data_packet bar;
		bar.a_data = i;
		get_abs_event(&foo, bar);

		if(ioctl(gamepad->fd, UI_SET_ABSBIT, foo.code) < 0) // Enables every axis required
			die("error: ioctl/setup_uinput");
	}

	struct uinput_user_dev uidev; // Device data
	memset(&uidev, 0, sizeof(uidev));

	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, dev_name); // Device name
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor  = 0x1;
	uidev.id.product = 0x1;
	uidev.id.version = 1;

	if(write(gamepad->fd, &uidev, sizeof(uidev)) < 0) // Writes the device data
		die("error: write/setup_uinput");

	if(ioctl(gamepad->fd, UI_DEV_CREATE) < 0) // And creates the device
		die("error: ioctl/setup_uinput");

	gamepad->status = 1;
}

void
destroy_uinput(struct uinput_controller *gamepad)
{
	if(ioctl(gamepad->fd, UI_DEV_DESTROY) < 0)
		die("error: ioctl/setup_uinput");

	gamepad->status = 0;
}

void
send_event(struct uinput_controller gamepad, struct input_event *ev)
{
	struct input_event sync;

	memset(&sync, 0, sizeof(struct input_event)); // Synchronization input action
	sync.type = EV_SYN;
	sync.code = 0;
	sync.value = 0;

	write(gamepad.fd, ev, sizeof(struct input_event));
	write(gamepad.fd, &sync, sizeof(struct input_event));
}
