/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

#ifndef serialjoy_defs_header
#define serialjoy_defs_header

#include <linux/input.h>
#include <termios.h> 

// Structs:

struct data_packet
{
	int type;
	unsigned char device;
	unsigned char a_data;
	unsigned char l_data;
	unsigned char h_data;
};

struct uinput_controller
{
	int status;
	int fd;
};

// simple_serial.c:

int open_port(char *, int);
int check_conn(int, int);
unsigned char read_char(int);
void print_char(int, unsigned char);
void read_packet(int, struct data_packet *);
void get_baud(char *, speed_t *);

// simple_uinput.c:

void open_uinput(struct uinput_controller *);
void setup_gamepad(struct uinput_controller *, char *);
void destroy_uinput(struct uinput_controller *);
void send_event(struct uinput_controller, struct input_event *);

// gamepad_events.c:

int get_key_event(struct input_event *, struct data_packet);
int get_abs_event(struct input_event *, struct data_packet);

// Useful aliases:

#define die(str, args...) do { \
perror(str); \
exit(EXIT_FAILURE); \
} while(0)

#define vprint(vlevel, format, ...)\
if(verbose_flag >= vlevel) fprintf(stdout, format, ##__VA_ARGS__); fflush(stdout);

// Parameters:

#define MAX_DEV 10 // maximum number of devices

#define MAX_NULL_CYCLE 20 // time to check = MAX_NULL_CYCLE * 0.5 seconds
#define MAX_CHECK_AFTER_FAIL 20 // timeout = MAX_CHECK_AFTER_FAIL * 0.5 seconds

#define DEFAULT_BAUD_RATE B38400

// Flags:

static int verbose_flag = 1,
	ignore_check_flag = 0,
	legacy_flag = 0,
	dry_run_flag = 0;

#endif
