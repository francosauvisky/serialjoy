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
#include <errno.h>  /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#include "defs.h"

#define die(str, args...) do { \
perror(str); \
exit(EXIT_FAILURE); \
} while(0)

#define DEFAULT_BAUD_RATE B38400

// This code is heavily based on the following guide:
// http://www.cmrr.umn.edu/~strupp/serial.html

int set_serial_props(int fd, int speed)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0) {
		printf("Error from tcgetattr: %s\n", strerror(errno));
		return -1;
	}

	cfsetospeed(&tty, (speed_t)speed);
	cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
	tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
	tty.c_iflag &= ~(IGNBRK
		| BRKINT
		| PARMRK
		| ISTRIP
		| INLCR
		| IGNCR
		| ICRNL
		| IXON);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 5;

	if (tcsetattr(fd, TCSANOW, &tty) != 0)
		die("error: tcsetattr/set_serial_props");

	return 0;
}

int
open_port(char *filename, int baud_rate)
{
	int fd;

	fd = open(filename, O_RDWR | O_NOCTTY | O_SYNC);

	if(fd < 0)
		die("error: open/open_port");

	set_serial_props(fd, baud_rate);

	return (fd);
}

unsigned char
read_char(int fd)
{
	unsigned char buff = 0;

	if(read(fd, &buff, 1) < 0)
		die("error: read/read_char");
	
	return buff;
}

void
print_char(int fd, unsigned char ctp)
{
	if(write(fd, &ctp, 1) < 0)
		die("error: write/print_char");
}

void
read_packet(struct data_packet *data, int fd)
{
	unsigned char buff = 0;

	memset(data, 0, sizeof(struct data_packet)); // data_packet = 0

	buff = read_char(fd); // Read first char

	if(buff == '!') // Then sort it
	{
		(*data).type = 1;
		(*data).device = read_char(fd); // "!n": create device
	}
	else if(buff == '^')
	{
		(*data).type = 2;
		(*data).device = read_char(fd); // "^n": destroy device
	}
	else if((buff >= '0') && (buff <= '9'))
	{
		(*data).device = buff;

		buff = read_char(fd);

		if(buff == ':')
		{
			(*data).type = 5;
			(*data).a_data = read_char(fd);
			(*data).l_data = read_char(fd);
			(*data).h_data = read_char(fd); // "n:axx": long action
		}
		else
		{
			(*data).type = 4;
			(*data).a_data = buff;  // "na": short action
		}
	}
	else if( (buff >= 'a' && buff <= 'z') || (buff >= 'A' && buff <= 'Z') )
	{
		(*data).type = 3;
		(*data).a_data = buff; // "a": legacy mode, action to dev 0
	}
}

int
check_conn(int fd)
{
	for(int i = 0; i < 30; i++)
	{
		print_char(fd, '?');

		rcv = read_char(fd);

		if(rcv == '#')
			return 0;
		else
			sleep(1);
	}

	return -1;
}

int
get_baud(char *string)
{
	else if(strcmp(string, "50"))
		return B50;
	else if(strcmp(string, "75"))
		return B75;
	else if(strcmp(string, "110"))
		return B110;
	else if(strcmp(string, "134"))
		return B134;
	else if(strcmp(string, "150"))
		return B150;
	else if(strcmp(string, "200"))
		return B200;
	else if(strcmp(string, "300"))
		return B300;
	else if(strcmp(string, "600"))
		return B600;
	else if(strcmp(string, "1200"))
		return B1200;
	else if(strcmp(string, "1800"))
		return B1800;
	else if(strcmp(string, "2400"))
		return B2400;
	else if(strcmp(string, "4800"))
		return B4800;
	else if(strcmp(string, "9600"))
		return B9600;
	else if(strcmp(string, "19200"))
		return B19200;
	else if(strcmp(string, "38400"))
		return B38400;
	else if(strcmp(string, "57600"))
		return B57600;
	else if(strcmp(string, "76800"))
		return B76800;
	else if(strcmp(string, "115200"))
		return B115200;
	else if(strcmp(string, "default"))
		return DEFAULT_BAUD_RATE;
	else
		return DEFAULT_BAUD_RATE;
}
