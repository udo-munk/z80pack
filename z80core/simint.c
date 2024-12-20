/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 */

/*
 *	This module contains interrupt handlers for POSIX OS'S:
 *
 *	int_on()	: initialize interrupt handlers
 *	int_off()	: reset interrupts to default
 *	user_int()	: handler for user interrupt (CNTL-C)
 *	quit_int()	: handler for signal "quit" (CNTL-\)
 *	term_int()	: handler for signal SIGTERM when process is killed
 */

#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simio.h"
#include "simint.h"

#include "unix_terminal.h"

static void user_int(int sig);
static void quit_int(int sig);
static void term_int(int sig);

void int_on(void)
{
	static struct sigaction newact;

	newact.sa_handler = user_int;
	sigemptyset(&newact.sa_mask);
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

	sigemptyset(&newact.sa_mask);
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
	UNUSED(sig);

	cpu_error = USERINT;
	cpu_state = ST_STOPPED;
}

static void quit_int(int sig)
{
	UNUSED(sig);

	cpu_error = USERINT;
	cpu_state = ST_STOPPED;
}

static void term_int(int sig)
{
	UNUSED(sig);

	exit_io();
	int_off();
	reset_unix_terminal();
	puts("\nKilled by user");
	exit(EXIT_SUCCESS);
}
