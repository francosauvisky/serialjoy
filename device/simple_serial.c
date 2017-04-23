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

#define PACKET_BUFFER_SIZE 5
#define BAUD_RATE B38400

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
open_port(char *filename)
{
	int fd;

	fd = open(filename, O_RDWR | O_NOCTTY | O_SYNC);

	if(fd < 0)
		die("error: open/open_port");

	set_serial_props(fd, BAUD_RATE);

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

	memset(data, 0, sizeof(struct data_packet));

	buff = read_char(fd);

	if(buff == '!')
	{
		(*data).type = 1;
		(*data).device = read_char(fd);
	}
	else if(buff == '^')
	{
		(*data).type = 2;
		(*data).device = read_char(fd);
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
			(*data).h_data = read_char(fd);
		}
		else
		{
			(*data).type = 4;
			(*data).a_data = buff;
		}
	}
	else if( (buff >= 'a' && buff <= 'z') || (buff >= 'A' && buff <= 'Z') )
	{
		(*data).type = 3;
		(*data).a_data = buff;
	}
}

int
check_conn(int fd)
{
	int nconn = 1, count = 0;;
	unsigned char rcv;

	while(nconn == 1)
	{
		print_char(fd, '?');

		rcv = read_char(fd);

		if(rcv == '#')
			nconn = 0;
		else
			sleep(1);

		if(++count == 30)
			nconn = -1;

	}

	return nconn;
}