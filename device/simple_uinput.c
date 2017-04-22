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

#define die(str, args...) do { \
perror(str); \
exit(EXIT_FAILURE); \
} while(0)

int
open_uinput()
{
	int fd;

	fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

	if(fd < 0)
	{
		fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
	}
	if(fd < 0)
		die("error: open/open_uinput");

	return fd;
}

void
setup_uinput(int ufd, char *dev_name)
{
	if(ioctl(ufd, UI_SET_EVBIT, EV_KEY) < 0)
		die("error: ioctl/setup_uinput");
	if(ioctl(ufd, UI_SET_EVBIT, EV_SYN) < 0)
		die("error: ioctl/setup_uinput");

	for(char i = 'A'; i <= 'Z'; i++)
	{
		struct input_event foo;
		get_event(&foo, i);

		if(ioctl(ufd, UI_SET_KEYBIT, foo.code) < 0)
			die("error: ioctl/setup_uinput");
	}

	if(ioctl(ufd, UI_SET_EVBIT, EV_ABS) < 0)
		die("error: ioctl/setup_uinput");
	if(ioctl(ufd, UI_SET_ABSBIT, ABS_X) < 0)
		die("error: ioctl/setup_uinput");
	if(ioctl(ufd, UI_SET_ABSBIT, ABS_Y) < 0)
		die("error: ioctl/setup_uinput");

	struct uinput_user_dev uidev;
	memset(&uidev, 0, sizeof(uidev));
	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, dev_name);
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor  = 0x1;
	uidev.id.product = 0x1;
	uidev.id.version = 1;

	if(write(ufd, &uidev, sizeof(uidev)) < 0)
		die("error: write/setup_uinput");
	ioctl(ufd, UI_DEV_CREATE);
}
