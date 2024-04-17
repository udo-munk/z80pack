/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2022 by Udo Munk
 *
 * This module contains the user interface for the Z80-CPU simulation,
 * here we just call the ICE.
 */

#include <unistd.h>
#include <termios.h>
#include "sim.h"
#include "simglb.h"

extern void ice_cmd_loop(int);

struct termios old_term;

/*
 *	The function "mon()" is the user interface, called
 *	from the simulation just after program start.
 */
void mon(void)
{
	tcgetattr(0, &old_term);

	ice_cmd_loop(x_flag);
}
