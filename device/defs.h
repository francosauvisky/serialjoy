/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

#ifndef gamepad_events_header
#define gamepad_events_header

#include <linux/input.h>
#include <termios.h> 

//simple_serial:

struct data_packet
{
	int type;
	unsigned char device;
	unsigned char a_data;
	unsigned char l_data;
	unsigned char h_data;
};

int open_port(char *, int);
int check_conn(int, int);
unsigned char read_char(int);
void print_char(int fd, unsigned char);
void read_packet(struct data_packet *, int);
void get_baud(char *, speed_t *);

//simple_uinput:

int open_uinput(void);
void setup_uinput(int, char *);
void destroy_uinput(int);

//gamepad_events:

int get_key_event(struct input_event *, struct data_packet);
int get_abs_event(struct input_event *, struct data_packet);

// Useful aliases:

#define die(str, args...) do { \
perror(str); \
exit(EXIT_FAILURE); \
} while(0)

// Parameters:

#define MAX_DEV 10 // maximum number of devices

#define MAX_NULL_CYCLE 20 // time to check = MAX_NULL_CYCLE * 0.5 seconds
#define MAX_CHECK_AFTER_FAIL 20 // timeout = MAX_CHECK_AFTER_FAIL * 0.5 seconds

#define DEFAULT_BAUD_RATE B38400

#endif
