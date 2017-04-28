/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

#include <stdio.h> /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h> /* String function definitions */
#include <unistd.h> /* UNIX standard function definitions */
#include <linux/input.h>
#include <getopt.h>

#include "defs.h"

#define die(str, args...) do { \
perror(str); \
exit(EXIT_FAILURE); \
} while(0)

#define vprint(vlevel, format, ...)\
if(verbose_flag >= vlevel) fprintf(stdout, format, ##__VA_ARGS__); fflush(stdout);

#define MAX_DEV 10

struct uinput_controller
{
	int status;
	int fd;
};

void print_help()
{

}
void print_usage()
{

}

static int verbose_flag = 1,
           ignore_check_flag = 0,
           legacy_flag = 0,
           auto_flag = 0,
           dry_run_flag = 0;

int
main(int argc, char *argv[])
{
	int serial_fd, runflag, nullcount = 0, check_flag = 0;
	speed_t baud_rate = 0;
	char *serial_tty = NULL;
	struct input_event ev, sync;
	struct uinput_controller gamepad[MAX_DEV];

	// ----- Options parsing

	if(argc <= 1)
		print_usage();

	while(1)
	{
		static struct option long_options[] =
		{
			{"verbose", no_argument, &verbose_flag, 2},
			{"silent", no_argument, &verbose_flag, 0},
			{"dry-run", no_argument, &dry_run_flag, 1},
			{"baud", required_argument, 0, 'b'},
			{"help", no_argument, 0, 'h'},
			{"ignore-check", no_argument, 0, 'i'},
			{"legacy", no_argument, 0, 'l'},
			{"port", required_argument, 0, 'p'},
			{0, 0, 0, 0}
		};

		// getopt_long stores the option index here.
		int option_index = 0;

		int c = getopt_long(argc, argv, "b:hilp:", long_options, &option_index);

		// Detect the end of the options.
		if (c == -1)
		{
			if((optind < argc) && !serial_tty) // If serial port is not defined
				// and there is an non-parsed option
			{
				serial_tty = malloc(strlen(argv[argc - 1]) + 1);
				serial_tty = argv[argc - 1]; // Last argument
			}

			break;
		}

		switch (c)
		{
			case 0:
				break;

			case 'b':
				get_baud(optarg, &baud_rate);
			break;

			case 'h':
				print_help();
			break;

			case 'i':
				ignore_check_flag = 1;
			break;

			case 'l':
				legacy_flag = 1;
			break;

			case 'p':
				if(!auto_flag)
				{
					serial_tty = malloc(strlen(optarg) + 1);
					strcpy(serial_tty, optarg);
				}
			break;

			case '?':
				print_usage();
			break;

			default:
				abort ();
		}
	}

	if(!serial_tty) // If serial port is not defined
		print_usage();

	if(!baud_rate)
		get_baud("default", &baud_rate);

	// ----- Done!

	// ----- Serial Port Initialization

	vprint(2, "Trying to open %s... ", serial_tty);

	if(access(serial_tty, F_OK) == -1) // Checks for file access
	{
		vprint(2, "Fail\n");
		fprintf(stderr, "Couldn't open port %s.\n", serial_tty);
		die("error: access/main");
	}

	serial_fd = open_port(serial_tty, baud_rate); // Opens the serial port

	vprint(2, "Done\n", serial_tty);

	// ----- Done

	if(dry_run_flag) // If --dry-run, then just checks and leaves
	{
		vprint(2, "[DRY-RUN] Checking for connection with adapter... ");
		if(check_conn(serial_fd, ignore_check_flag) == 0)
		{
			vprint(1, "Done\n");
			exit(EXIT_SUCCESS);
		}
		else
		{
			vprint(1, "Fail\n");
			exit(EXIT_FAILURE);
		}
	}

	vprint(2, "Trying to open uinput... ");

	for(int i = 0; i < MAX_DEV; i++) // Initializes the gamepad struct array
	{
		gamepad[i].status = 0;
		gamepad[i].fd = open_uinput();
	}

	memset(&sync, 0, sizeof(struct input_event)); // Synchronization input action
	sync.type = EV_SYN;
	sync.code = 0;
	sync.value = 0;

	vprint(2, "Done\n");

	vprint(2, "Checking connection with adapter... ");

	if(check_conn(serial_fd, ignore_check_flag) == 0)
	{
		vprint(2, "Done\n");
		usleep(250000);
		print_char(serial_fd, 'd');
	}
	else
	{
		vprint(2, "Fail\n");
		fprintf(stderr, "error: check_conn/main\n");
		exit(EXIT_FAILURE);
	}

	vprint(2, "Running main loop:\n");

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
				vprint(1, "Created device %s\n", dev_name);
			}
		}
		else if(dpkg.type == 2 && dpkg.device >= '0' && dpkg.device <= '9') // Device destroy
		{
			int device_n = dpkg.device - '0';
			char dev_name[20];

			if(gamepad[device_n].status == 1)
			{
				sprintf(dev_name, "serialjoy%01d", device_n);

				destroy_uinput(gamepad[device_n].fd);
				gamepad[device_n].status = 0;

				vprint(1, "Destroyed device %s\n", dev_name);
			}
		}
		else if(dpkg.type == 3 && dpkg.a_data != 0) // Send data to device 0
		{
			int device_n = 0;

			if(gamepad[device_n].status == 1 && get_event(&ev, dpkg.a_data) == 0)
			{
				write(gamepad[device_n].fd, &ev, sizeof(struct input_event));
				write(gamepad[device_n].fd, &sync, sizeof(struct input_event));
				vprint(2, "data: type 3 dev %c btn %c val %d\n", dpkg.device, dpkg.a_data < 'a'?
					dpkg.a_data : (dpkg.a_data - 'a'+'A'), dpkg.a_data >= 'a');
			}
			else
			{
				vprint(2, "Invalid type 3 packet received");
			}
		}
		else if(dpkg.type == 4 && dpkg.device >= '0'
			&& dpkg.device <= '9' && dpkg.a_data != 0) // Send data to device dpkg.device
		{
			int device_n = dpkg.device - '0';

			if(gamepad[device_n].status == 1 && get_event(&ev, dpkg.a_data) == 0)
			{
				write(gamepad[device_n].fd, &ev, sizeof(struct input_event));
				write(gamepad[device_n].fd, &sync, sizeof(struct input_event));

				vprint(2, "data: type 4 dev %c btn %c val %d\n", dpkg.device, dpkg.a_data < 'a'?
					dpkg.a_data : (dpkg.a_data - 'a'+'A'), dpkg.a_data >= 'a');
			}
			else
			{
				vprint(2, "Invalid type 4 packet received");
			}
		}
		else if(dpkg.type == 5 && dpkg.device >= '0'
			&& dpkg.device <= '9' && dpkg.a_data != 0
			&& dpkg.l_data != 0 && dpkg.h_data != 0) // Send complex data to device dpkg.device
		{
			vprint(2, "Type 5 packet received [UNIMPLEMENTED]\n");
		}
		else // Not received any valid data
		{
			nullcount++;

			if(nullcount > 10)
			{
				if(check_conn(serial_fd, ignore_check_flag) != 0)
				{
					fprintf(stderr, "error: check_conn/main\n");
					exit(EXIT_FAILURE);
				}
				nullcount = 0;
			}
		}

		if(dpkg.type != 0)
			nullcount = 0;
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

/*
Options sketch:

-h or --help: Prints help (to be written)

--dry-run: Only checks if adapter is connected

-p (tty) or --port (tty): Connect to the device (tty)
For example: --port /dev/ttyUSB0
This option is ignored if --auto is set

-b (baud) or --baud (baud): Sets the baud rate for the serial connection
Check the possible values!

-l or --legacy: Go to legacy mode (init device 0 automatically)

-i or --ignore-check: Don't check the connection from the adapter

--verbose: Prints every information received from the adapter

--silent: Don't print anything besides errors
*/
