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
#include <linux/input.h>

#include "defs.h"

#define die(str, args...) do { \
perror(str); \
exit(EXIT_FAILURE); \
} while(0)

#define MAX_DEV 4

int
main(int argc, char *argv[])
{
	int serial_fd, uinput_fd[MAX_DEV] = {0};
	struct input_event ev, sync;

	if(access(argv[argc - 1], F_OK) == -1)
		die("error: access/main");

	serial_fd = open_port(argv[argc - 1]);

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


// uinput_fd = open_uinput();
// setup_uinput(uinput_fd, "serialjoy");

/*
# Communications protocol sketch:

- Valid characters: Every printable character (0x20 up to 0x7E)
- Packets size: Strings from 2 to 8 chars. If more is need, then send multi-packets signals.

## Packet formats:

- From adapter to device:

"$": Return "#" for handshake
"!n", where n = [0..9]: Create device n
"^n", where n = [0..9]: Destroy device n
"a", where a = [a..zA..Z]: Send simple action a to device 0
"na", where n = [0..9] and a = [a..zA..Z]: Send simple action a to device n
"n:axx" where n = [0..9], a = [a..zA..Z] and each x = [0x40..0x5F]: Send complex action xxxx to device n
"s" where s is a null-terminated string: Returning preprogrammed string (only if device asks)

- From device to adapter:

"&": Return "%" for handshake
"d": Force to create devices
"v": Return preprogrammed string

## Simple actions:

[a..z]: Buttons presses (up to 26 buttons)
[A..Z]: Buttons relase (up to 26 buttons)

## Complex actions "axxx":

- First char:

[a..zA..Z]: Action code (defined afterwards), for example analog axis

- Second and third chars:

[0x40..0x5F][0x40..0x5F]: Action data (first 5 bits of each char) = 10 bits
*/