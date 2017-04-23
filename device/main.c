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

#define MAX_DEV 10
// #define LEGACY

struct uinput_controller
{
	int status;
	int fd;
};

int
main(int argc, char *argv[])
{
	int serial_fd, runflag, nullcount = 0, check_flag = 0;
	struct input_event ev, sync;
	struct uinput_controller gamepad[MAX_DEV];

	// ----- Serial Port Initialization

	if(access(argv[argc - 1], F_OK) == -1) // Checks for file access
		die("error: access/main");

	serial_fd = open_port(argv[argc - 1]); // Opens the serial port

	// ----- Done

	for(int i = 0; i < MAX_DEV; i++) // Initializes the gamepad struct array
	{
		gamepad[i].status = 0;
		gamepad[i].fd = open_uinput();
	}

	memset(&sync, 0, sizeof(struct input_event)); // Synchronization input action
	sync.type = EV_SYN;
	sync.code = 0;
	sync.value = 0;

	if(check_conn(serial_fd) == 0)
	{
		check_flag = 1;
	}
	else
	{
		fprintf(stderr, "warning: check_conn/main\n");
	}

	runflag = 1;
	while(runflag == 1)
	{
		struct data_packet dpkg;
		read_packet(&dpkg, serial_fd); // Reads the data from serial port

		if(dpkg.type == 1 && dpkg.device >= '0' && dpkg.device <= '9') // Device create
		{
			int device_n = dpkg.device - '0';
			char dev_name[20];

			if(gamepad[device_n].status == 0)
			{
				sprintf(dev_name, "serialjoy%01d", device_n);
				setup_uinput(gamepad[device_n].fd, dev_name);
				gamepad[device_n].status = 1;
			}
		}
		else if(dpkg.type == 2 && dpkg.device >= '0' && dpkg.device <= '9') // Device destroy
		{
			int device_n = dpkg.device - '0';

			if(gamepad[device_n].status == 1)
			{
				destroy_uinput(gamepad[device_n].fd);
				gamepad[device_n].status = 0;
			}
		}
		else if(dpkg.type == 3 && dpkg.a_data != 0) // Send data to device 0
		{
			int device_n = 0;

			if(gamepad[device_n].status == 1 && get_event(&ev, dpkg.a_data) == 0)
			{
				write(gamepad[device_n].fd, &ev, sizeof(struct input_event));
				write(gamepad[device_n].fd, &sync, sizeof(struct input_event));
			}

			#ifdef LEGACY
			else if(gamepad[device_n].status == 0 && get_event(&ev, dpkg.a_data) == 0)
			{
				char dev_name[20];
				sprintf(dev_name, "serialjoy%d", device_n);
				setup_uinput(gamepad[device_n].fd, dev_name);
				gamepad[device_n].status = 1;

				write(gamepad[device_n].fd, &ev, sizeof(struct input_event));
				write(gamepad[device_n].fd, &sync, sizeof(struct input_event));
			}
			#endif
		}
		else if(dpkg.type == 4 && dpkg.device >= '0'
			&& dpkg.device <= '9' && dpkg.a_data != 0) // Send data to device dpkg.device
		{
			int device_n = dpkg.device - '0';

			if(gamepad[device_n].status == 1 && get_event(&ev, dpkg.a_data) == 0)
			{
				write(gamepad[device_n].fd, &ev, sizeof(struct input_event));
				write(gamepad[device_n].fd, &sync, sizeof(struct input_event));
			}
		}
		else if(dpkg.type == 5 && dpkg.device >= '0'
			&& dpkg.device <= '9' && dpkg.a_data != 0
			&& dpkg.l_data != 0 && dpkg.h_data != 0) // Send complex data to device dpkg.device
		{
			// Still unimplemented
		}
		else // Not received any valid data
		{
			nullcount++;

			if(nullcount > 10)
			{
				if(check_conn(serial_fd) != 0)
				{
					fprintf(stderr, "error: check_conn/main\n");
					exit(EXIT_FAILURE);
				}
				nullcount = 0;
				check_flag = 1;
			}
		}

		if(dpkg.type != 0)
			nullcount = 0;

		if(check_flag == 1)
		{
			print_char(serial_fd, 'd');
			check_flag = 0;
		}
	}
}

/*
# Communications protocol sketch:

- Valid characters: Every printable character (0x20 up to 0x7E)
- Packets size: Strings from 2 to 8 chars. If more is need, then send multi-packets signals.

## Packet formats:

- From adapter to device:

1- "!n", where n = [0..9]: Create device n
2- "^n", where n = [0..9]: Destroy device n
3- "a", where a = [a..zA..Z]: Send simple action a to device 0
4- "na", where n = [0..9] and a = [a..zA..Z]: Send simple action a to device n
5- "n:axx" where n = [0..9], a = [a..zA..Z] and each x = [0x40..0x5F]: Send complex action xxxx to device n
6- "s" where s is a null-terminated string: Returning preprogrammed string (only if device asks)

- From device to adapter:

"d": Force to create devices
"v": Return preprogrammed string

- Handshake protocol

"#": OK
"%": Not OK
"?": Are you there? (return OK)

When adapter sends "!n", device must answer OK when sucessful.
If adapter doesn't responds to "?", then go to legacy mode

## Simple actions:

[a..z]: Buttons presses (up to 26 buttons)
[A..Z]: Buttons relase (up to 26 buttons)

## Complex actions "axxx":

- First char:

[a..zA..Z]: Action code (defined afterwards), for example analog axis

- Second and third chars:

[0x40..0x5F][0x40..0x5F]: Action data (first 5 bits of each char) = 10 bits
*/