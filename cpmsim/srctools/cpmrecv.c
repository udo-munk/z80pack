/*
 * Receive a file out of the named pipe "auxout" from CP/M simulation
 *
 * Copyright (C) 1988-2017 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * History:
 * 05-OKT-1988 Development on TARGON/35 with AT&T Unix System V.3
 * 11-MAR-1993 comments in english and ported to COHERENT 4.0
 * 01-OCT-2006 modified to compile on modern POSIX OS's
 * 09-MAR-2016 moved pipes to /tmp/.z80pack
 * 11-MAY-2016 delayed create outfile so that it is created if used
 * 20-MAR-2017 renamed pipe
 * 19-APR-2024 don't use exit() in signal handler and switch to sigaction()
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>

#define UNUSED(x) (void)(x)

int signal_catched;

int main(int argc, char *argv[])
{
	char c;
	int fdin, fdout;
	static struct sigaction newact;
	void int_handler(int);

	fdout = 0;

	if (argc != 2) {
		puts("usage: cpmrecv filename &");
		exit(EXIT_FAILURE);
	}
	if ((fdin = open("/tmp/.z80pack/cpmsim.auxout", O_RDONLY)) == -1) {
		perror("pipe auxout");
		exit(EXIT_FAILURE);
	}

	newact.sa_handler = SIG_IGN;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGINT, &newact, NULL);
	sigaction(SIGQUIT, &newact, NULL);
	newact.sa_handler = int_handler;
	sigaction(SIGHUP, &newact, NULL);

	while (!signal_catched && read(fdin, &c, 1) == 1) {
		if (c != '\r') {
			if (fdout == 0) {
				if ((fdout = creat(argv[1], 0644)) == -1) {
					perror(argv[1]);
					exit(EXIT_FAILURE);
				}
			}
			write(fdout, &c, 1);
		}
	}

	close(fdin);
	if (fdout)
		close(fdout);
	return (EXIT_SUCCESS);
}

void int_handler(int sig)
{
	UNUSED(sig);

	signal_catched = 1;
}
