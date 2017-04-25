/*
This file is part of serialjoy.

(c) Franco Sauvisky <francosauvisky@gmail.com>

This source file is subject to the 3-Clause BSD License that is bundled
with this source code in the file LICENSE.md
*/

struct genesis_controller
{
	uint8_t conf;
	uint16_t state;
	uint16_t old_state;

	volatile uint8_t *input_ddr;
	volatile uint8_t *input_pin;
	volatile uint8_t *input_port;

	volatile uint8_t *sel_ddr;
	volatile uint8_t *sel_port;

	uint8_t pins_d[6];
	uint8_t pin_sel;
};

void genesis_reset();

void genesis_setup(struct genesis_controller *);

void genesis_read_state(struct genesis_controller *);

void genesis_update_state(struct genesis_controller *);