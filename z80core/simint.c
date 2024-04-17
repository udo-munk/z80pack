/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
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
	UNUSED(sig);

	cpu_error = USERINT;
	cpu_state = STOPPED;
}

static void quit_int(int sig)
{
	UNUSED(sig);

	cpu_error = USERINT;
	cpu_state = STOPPED;
}

static void term_int(int sig)
{
	UNUSED(sig);

	exit_io();
	int_off();
	tcsetattr(0, TCSADRAIN, &old_term);
	puts("\nKilled by user");
	exit(0);
}
