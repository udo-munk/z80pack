/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2018 by Udo Munk
 *
 * History:
 * 28-SEP-87 Development on TARGON/35 with AT&T Unix System V.3
 * 11-JAN-89 Release 1.1
 * 08-FEB-89 Release 1.2
 * 13-MAR-89 Release 1.3
 * 09-FEB-90 Release 1.4  Ported to TARGON/31 M10/30
 * 20-DEC-90 Release 1.5  Ported to COHERENT 3.0
 * 10-JUN-92 Release 1.6  long casting problem solved with COHERENT 3.2
 *			  and some optimisation
 * 25-JUN-92 Release 1.7  comments in english and ported to COHERENT 4.0
 * 02-OCT-06 Release 1.8  modified to compile on modern POSIX OS's
 * 18-NOV-06 Release 1.9  modified to work with CP/M sources
 * 08-DEC-06 Release 1.10 modified MMU for working with CP/NET
 * 17-DEC-06 Release 1.11 TCP/IP sockets for CP/NET
 * 25-DEC-06 Release 1.12 CPU speed option
 * 19-FEB-07 Release 1.13 various improvements
 * 06-OCT-07 Release 1.14 bug fixes and improvements
 * 06-AUG-08 Release 1.15 many improvements and Windows support via Cygwin
 * 25-AUG-08 Release 1.16 console status I/O loop detection and line discipline
 * 20-OCT-08 Release 1.17 frontpanel integrated and Altair/IMSAI emulations
 * 24-JAN-14 Release 1.18 bug fixes and improvements
 * 02-MAR-14 Release 1.19 source cleanup and improvements
 * 14-MAR-14 Release 1.20 added Tarbell SD FDC and printer port to Altair
 * 29-MAR-14 Release 1.21 many improvements
 * 29-MAY-14 Release 1.22 improved networking and bugfixes
 * 04-JUN-14 Release 1.23 added 8080 emulation
 * 06-SEP-14 Release 1.24 bugfixes and improvements
 * 18-FEB-15 Release 1.25 bugfixes, improvements, added Cromemco Z-1
 * 18-APR-15 Release 1.26 bugfixes and improvements
 * 18-JAN-16 Release 1.27 bugfixes and improvements
 * 05-MAY-16 Release 1.28 improved usability
 * 20-NOV-16 Release 1.29 bugfixes and improvements
 * 15-DEC-16 Release 1.30 improved memory management, machine cycle correct CPUs
 * 28-DEC-16 Release 1.31 improved memory management, reimplemented MMUs
 * 12-JAN-17 Release 1.32 improved configurations, front panel, added IMSAI VIO
 * 07-FEB-17 Release 1.33 bugfixes, improvements, better front panels
 * 16-MAR-17 Release 1.34 improvements, added ProcTec VDM-1
 * 03-AUG-17 Release 1.35 added UNIX sockets, bugfixes, improvements
 * 21-DEC-17 Release 1.36 bugfixes and improvements
 */

/*
 *	This module contain the interrupt handlers for the OS:
 *
 *	int_on()	: initialise interrupt handlers
 *	int_off()	: reset interrupts to default
 *	user_int()	: handler for user interrupt (CNTL-C)
 *	quit_int()	: handler for signal "quit" (CNTL-\)
 *	term_int()	: handler for signal SIGTERM when process is killed
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include "sim.h"
#include "simglb.h"

static void user_int(int), quit_int(int), term_int(int);
extern void exit_io(void);
extern struct termios old_term;

void int_on(void)
{
	static struct sigaction newact;

	newact.sa_handler = user_int;
	memset((void *) &newact.sa_mask, 0, sizeof(newact.sa_mask));
	newact.sa_flags = 0;
	sigaction(SIGINT, &newact, NULL);
	newact.sa_handler = quit_int;
	sigaction(SIGQUIT, &newact, NULL);
	newact.sa_handler = term_int;
	sigaction(SIGTERM, &newact, NULL);
}

void int_off(void)
{
	static struct sigaction newact;

	memset((void *) &newact.sa_mask, 0, sizeof(newact.sa_mask));
	newact.sa_flags = 0;
	newact.sa_handler = SIG_IGN;
	sigaction(SIGALRM, &newact, NULL);
	newact.sa_handler = SIG_DFL;
	sigaction(SIGINT, &newact, NULL);
	sigaction(SIGQUIT, &newact, NULL);
	sigaction(SIGTERM, &newact, NULL);
}

static void user_int(int sig)
{
	sig = sig;	/* to avoid compiler warning */

	cpu_error = USERINT;
	cpu_state = STOPPED;
}

static void quit_int(int sig)
{
	sig = sig;	/* to avoid compiler warning */

	cpu_error = USERINT;
	cpu_state = STOPPED;
}

static void term_int(int sig)
{
	sig = sig;	/* to avoid compiler warning */

	exit_io();
	int_off();
	tcsetattr(0, TCSADRAIN, &old_term);
	puts("\nKilled by user");
	exit(0);
}
