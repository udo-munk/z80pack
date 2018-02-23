/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2017 by Udo Munk
 *
 * This module contains initialisation and reset functions for
 * the POSIX/BSD line discipline, so that stdin/stdout can be used
 * as terminal for ancient machines.
 *
 * History:
 * 24-SEP-08 first version finished
 * 16-JAN-14 discard input at reset
 * 15-APR-14 added some more c_cc's used on BSD systems
 * 24-FEB-17 set line discipline only if fd 0 is a tty
 */

#include <unistd.h>
#include <termios.h>

struct termios old_term, new_term;

static int init_flag;

void set_unix_terminal(void)
{
	if (init_flag || !isatty(0))
		return;

	tcgetattr(0, &old_term);
	new_term = old_term;

	new_term.c_lflag &= ~(ICANON | ECHO);
	new_term.c_iflag &= ~(IXON | IXANY | IXOFF);
	new_term.c_iflag &= ~(IGNCR | ICRNL | INLCR);
	new_term.c_oflag &= ~(ONLCR | OCRNL);

	/* these are common for all UNIX's nowadays */
	new_term.c_cc[VMIN] = 1;
	new_term.c_cc[VINTR] = 0;
	new_term.c_cc[VSUSP] = 0;

	/* some newer ones from BSD's */
#ifdef VDSUSP
	new_term.c_cc[VDSUSP] = 0;
#endif
#ifdef VDISCARD
	new_term.c_cc[VDISCARD] = 0;
#endif
#ifdef VLNEXT
	new_term.c_cc[VLNEXT] = 0;
#endif

	tcsetattr(0, TCSADRAIN, &new_term);

	init_flag++;
}

void reset_unix_terminal(void)
{
	if (!init_flag || !isatty(0))
		return;

	tcsetattr(0, TCSAFLUSH, &old_term);

	init_flag--;
}
