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
#include <signal.h>

#include "defs.h"

// ######### Parameters define:

#define MAX_DEV 10
#define MAX_NULL_CYCLE 10 // time to check = MAX_NULL_CYCLE * 0.5 seconds

// ######### Some useful aliases:

#define die(str, args...) do { \
perror(str); \
exit(EXIT_FAILURE); \
} while(0)

#define vprint(vlevel, format, ...)\
if(verbose_flag >= vlevel) fprintf(stdout, format, ##__VA_ARGS__); fflush(stdout);

// ######### Signal handling:

volatile int sigint_stop = 0;

volatile int sigint_serial_fd = 0;

void sigint_handler(int sig)
{
	if(sigint_serial_fd) // Stops any read cycle freezing the main loop
	{
		struct termios tty;

		if (tcgetattr(sigint_serial_fd, &tty) < 0)
			die("error: tcgetattr/sigint_handler");

		tty.c_cc[VMIN] = 0;
		tty.c_cc[VTIME] = 0;

		if (tcsetattr(sigint_serial_fd, TCSANOW, &tty) != 0)
			die("error: tcsetattr/sigint_handler");
	}

	sigint_stop = 1;
}

// ######### Printing functions

void print_help()
{
	fprintf(stdout,
"\n Usage: ./serialjoy [\e[4moptions\e[0m] \e[4mserial_port\e[0m\n\
       ./serialjoy [\e[4moptions\e[0m] [\e[1m--port\e[0m|\e[1m-p\e[0m] \e[4mserial_port\e[0m\n\
\n\
    Serialjoy is a universal and hobbist-friendly adapter of joysticks/gamepads\n\
    on Linux using serial ports.\n\
\n\
 Options:\n\
      \e[1m-p\e[0m \e[4mserial_port\e[0m\n\
      \e[1m--port\e[0m \e[4mserial_port\e[0m: Use the device file \e[4mserial_port\e[0m for communicating\n\
                          with the adapter. If not defined, uses the last\n\
                          argument given.\n\
\n\
      \e[1m-b\e[0m \e[4mbaud_rate\e[0m\n\
      \e[1m--baud\e[0m \e[4mbaud_rate\e[0m: Sets \e[4mbaud_rate\e[0m as the baud rate of the serial port\n\
                        The availiable values for baud rate are:\n\
                        50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800,\n\
                        2400, 4800, 9600, 19200, 38400, 57600, 115200\n\
                        If no baud rate is specified, then use 38400.\n\
\n\
      \e[1m--verbose\e[0m: Shows all that is being done while running, including\n\
                 all the data received from the adapter.\n\
\n\
      \e[1m--silent\e[0m: Prints only errors\n\
\n\
      \e[1m--dry-run\e[0m: Does not start the main loop, only opens the serial port\n\
                 and checks for the device, returning 0 if sucess or 1 if\n\
                 fails. Useful for detecting in which port the adapter is\n\
                 connected.\n\
\n\
      \e[1m-l\e[0m\n\
      \e[1m--legacy\e[0m:  Initializes devices automatically\n\
\n\
      \e[1m-i\e[0m\n\
      \e[1m-ignore-check\e[0m: Don't wait for the adapter to respond.\n\
\n\
 Example:\n\
      ./serialjoy --verbose --baud 38400 --port /dev/ttyUSB0\n\
        Connects to the adapter at /dev/ttyUSB0 with baud rate 38400.\n\
\n\
 See the documentation at https://github.com/francosauvisky/serialjoy for\n\
 more information\n\
\n\
"
);

	exit(EXIT_SUCCESS);
}

void print_usage()
{
	fprintf(stderr,
"Usage: ./serialjoy [\e[4moptions\e[0m] \e[4mserial_port\e[0m\n\
       where \e[4mserial_port\e[0m is the path of the serial port device file\n\
\n\
Example: ./serialjoy --baud 38400 /dev/ttyUSB0\n\
\n\
See ./serialjoy --help for more information\n\
"
);

	exit(EXIT_FAILURE);
}

// ######### Flags and structs:

static int verbose_flag = 1,
ignore_check_flag = 0,
legacy_flag = 0,
auto_flag = 0,
dry_run_flag = 0;

struct uinput_controller
{
	int status;
	int fd;
};

// Main code:

int
main(int argc, char *argv[])
{
	int serial_fd, runflag, nullcount = 0, check_flag = 0;
	speed_t baud_rate = 0;
	char *serial_tty = NULL;
	struct input_event ev, sync;
	struct uinput_controller gamepad[MAX_DEV];

	// ################ Signal handling ################

	signal(SIGINT, sigint_handler);

	// ################ Options parsing ################

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

	if(!baud_rate) // If baud rate is not defined
		get_baud("default", &baud_rate);

	// ################ Serial Port Initialization ################

	vprint(2, "Trying to open %s... ", serial_tty);

	if(access(serial_tty, F_OK) == -1) // Checks for file access
	{
		vprint(2, "Fail\n");
		fprintf(stderr, "Couldn't open port %s.\n", serial_tty);
		die("error: access/main");
	}

	serial_fd = open_port(serial_tty, baud_rate); // Opens the serial port

	sigint_serial_fd = serial_fd; // For terminating the program

	vprint(2, "Done\n", serial_tty);

	// ################ Dry run ################

	if(dry_run_flag) // If --dry-run, then just checks and leaves
	{
		vprint(2, "[DRY-RUN] Checking for connection with adapter... ");

		int check_success = 0;

		for(int j = 0; j < 2; j++)
		{
			print_char(serial_fd, '?');

			usleep(100000);
			for(int i = 0; i < 3; i++)
			{
				unsigned char rcv = read_char(serial_fd);

				if(rcv == '#')
				{
					check_success = 1;
					break;
				}
			}
		}

		if(check_success == 1)
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

	// ################ uinput initialization ################

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

	// ################ Connection check ################

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

	// ################ Main loop ################
	// (mostly: read from serial port, process data, send to uinput)

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
			char dev_name[20];

			if((get_event(&ev, dpkg.a_data) == 0) && gamepad[device_n].status == 1)
			{
				write(gamepad[device_n].fd, &ev, sizeof(struct input_event));
				write(gamepad[device_n].fd, &sync, sizeof(struct input_event));
				vprint(2, "data: type 3 dev %c btn %c val %d\n", dpkg.device, dpkg.a_data < 'a'?
					dpkg.a_data : (dpkg.a_data - 'a'+'A'), dpkg.a_data >= 'a');
			}
			else if(gamepad[device_n].status == 0 && legacy_flag == 1)
			{
				sprintf(dev_name, "serialjoy%01d", device_n);
				setup_uinput(gamepad[device_n].fd, dev_name);
				gamepad[device_n].status = 1;
				vprint(1, "Created device %s (legacy mode)\n", dev_name);

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
			char dev_name[20];

			if(gamepad[device_n].status == 1 && get_event(&ev, dpkg.a_data) == 0)
			{
				write(gamepad[device_n].fd, &ev, sizeof(struct input_event));
				write(gamepad[device_n].fd, &sync, sizeof(struct input_event));

				vprint(2, "data: type 4 dev %c btn %c val %d\n", dpkg.device, dpkg.a_data < 'a'?
					dpkg.a_data : (dpkg.a_data - 'a'+'A'), dpkg.a_data >= 'a');
			}
			else if(gamepad[device_n].status == 0 && legacy_flag == 1)
			{
				sprintf(dev_name, "serialjoy%01d", device_n);
				setup_uinput(gamepad[device_n].fd, dev_name);
				gamepad[device_n].status = 1;
				vprint(1, "Created device %s (legacy mode)\n", dev_name);

				write(gamepad[device_n].fd, &ev, sizeof(struct input_event));
				write(gamepad[device_n].fd, &sync, sizeof(struct input_event));
				vprint(2, "data: type 3 dev %c btn %c val %d\n", dpkg.device, dpkg.a_data < 'a'?
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

			if(nullcount > MAX_NULL_CYCLE)
			{
				if(check_conn(serial_fd, ignore_check_flag) != 0)
				{
					fprintf(stderr, "error: check_conn/main\n");
					exit(EXIT_FAILURE);
				}
				nullcount = 0;
			}
		}

		if(dpkg.type != 0) // Clears counter if data is received
			nullcount = 0;

		if(sigint_stop) // If sigint, then stop run
			break;
	}

	// ################ Terminating devices ################

	vprint(1, "\n");

	for(int i = 0; i < MAX_DEV; i++) // Initializes the gamepad struct array
	{
		if(gamepad[i].status == 1)
		{
			char dev_name[20];
			sprintf(dev_name, "serialjoy%01d", i);

			vprint(1, "Destroying device %s... ", dev_name);

			destroy_uinput(gamepad[i].fd);
			gamepad[i].status = 0;

			vprint(1, "Done\n", dev_name);
		}

		close(gamepad[i].fd);
	}

	vprint(2, "Closing serial port... ");
	close(serial_fd);
	vprint(2, "Done\n");

	return 0;
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

-b (baud) or --baud (baud): Sets the baud rate for the serial connection
Possible values: 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400,
4800, 9600, 19200, 38400, 57600, 115200

-l or --legacy: Go to legacy mode (init device 0 automatically)

-i or --ignore-check: Don't check the connection from the adapter

--verbose: Prints every information received from the adapter

--silent: Don't print anything besides errors
*/
