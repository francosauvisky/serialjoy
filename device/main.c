/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.
*/

#include <stdio.h>  /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h> /* String function definitions */
#include <unistd.h> /* UNIX standard function definitions */
#include <linux/input.h>

#include "defs.h"

#define die(str, args...) do { \
perror(str); \
exit(EXIT_FAILURE); \
} while(0)

int
main(int argc, char *argv[])
{
	int serial_fd, uinput_fd;

	if(access(argv[argc - 1], F_OK) == -1)
		die("error: access/main");

	serial_fd = open_port(argv[argc - 1]);
	uinput_fd = open_uinput();
	setup_uinput(uinput_fd, "serialjoy");

	struct input_event ev, sync;

	memset(&sync, 0, sizeof(struct input_event));
	sync.type = EV_SYN;
	sync.code = 0;
	sync.value = 0;

	while(1){
		unsigned char action_char = read_char(serial_fd);

		if(get_event(&ev, action_char) == 0)
		{
			write(uinput_fd, &ev, sizeof(struct input_event));
			write(uinput_fd, &sync, sizeof(struct input_event));
		}
	}
}
