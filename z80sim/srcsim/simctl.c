/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 *
 * This module contains the user interface for the Z80-CPU simulation,
 * here we just call the ICE.
 */

#include <unistd.h>
#include <termios.h>
#include "sim.h"
#include "simglb.h"
#include "../../iodevices/unix_terminal.h"

extern void ice_cmd_loop(int);

/*
 *	The function "mon()" is the user interface, called
 *	from the simulation just after program start.
 */
void mon(void)
{
	ice_before_go = set_unix_terminal;
	ice_after_go = reset_unix_terminal;
	ice_cmd_loop(x_flag);
}
