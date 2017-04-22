/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

void setup_uart(void);

void flush_UART_RX_buffer(void);

unsigned char read_nb_char(void);

unsigned char read_b_char(void);

void uart_print(const char *);

void uart_print_bin(uint8_t);

void uart_print_long_bin(uint16_t);
