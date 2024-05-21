/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * This module contains the user interface for the Z80-CPU simulation,
 * here we just call the ICE.
 *
 * History:
 * 15-SEP-2019 (Mike Douglas) Created from simctl.c in the z80sim
 *	       source directory. Added unix_terminal code to emulate
 *	       console I/O for the Mostek AID-80F and SYS-80FT computers
 * 27-SEP-2019 (Udo Munk) fix double loading of ROM
 * 30-SEP-2019 (Mike Douglas) accept also upper case
 */

#include <stdlib.h>
#include <stdio.h>
#include "sim.h"
#include "simglb.h"
#include "mostek-fdc.h"
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

	printf("Type CTRL-\\ to halt emulation\n\n");

	fdc_reset();

	ice_cmd_loop(x_flag ? 2 : 0);
}
