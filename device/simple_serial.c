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

int
open_port(char *filename)
{
	int fd;

	fd = open(filename, O_RDWR | O_NOCTTY | O_NDELAY);

	if(fd < 0)
		die("error: open/open_port");
	
	fcntl(fd, F_SETFL, 0);

	struct termios options;

	tcgetattr(fd, &options);

	cfsetispeed(&options, B38400);
	cfsetospeed(&options, B38400);
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	tcsetattr(fd, TCSANOW, &options);

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
