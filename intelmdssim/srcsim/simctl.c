/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * This module contains the user interface an Intel MDS-800 system,
 * for now we just call the ICE.
 *
 * History:
 */

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include "sim.h"
#include "simglb.h"
#include "unix_terminal.h"

extern void ice_cmd_loop(int);

/*
 *	The function "mon()" is the user interface, called
 *	from the simulation just after program start.
 */
void mon(void)
{
	ice_before_go = set_unix_terminal;
	ice_after_go = reset_unix_terminal;
	atexit(reset_unix_terminal);

	ice_cmd_loop(x_flag);
}
