/*
 * Receive a file out of the named pipe "auxout" from CP/M simulation
 *
 * Copyright (C) 1988-2017 by Udo Munk
 *
 * History:
 * 05-OKT-1988 Development on TARGON/35 with AT&T Unix System V.3
 * 11-MAR-1993 comments in english and ported to COHERENT 4.0
 * 01-OCT-2006 modified to compile on modern POSIX OS's
 * 09-MAR-2016 moved pipes to /tmp/.z80pack
 * 11-MAY-2016 delayed create outfile so that it is created if used
 * 20-MAR-2017 renamed pipe
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>

#define UNUSED(x) (void)(x)

int fdin, fdout;

int main(int argc, char *argv[])
{
	char c;
	void int_handler(int);

	fdout = 0;

	if (argc != 2) {
		puts("usage: cpmrecv filename &");
		exit(1);
	}
	if ((fdin = open("/tmp/.z80pack/cpmsim.auxout", O_RDONLY)) == -1) {
		perror("pipe auxout");
		exit(1);
	}

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGHUP, int_handler);

	for (;;) {
		if (read(fdin, &c, 1) == 1) {
			if (c != '\r') {
				if (fdout == 0) {
				  if ((fdout = creat(argv[1], 0644)) == -1) {
					perror(argv[1]);
					exit(1);
				  }
				}
				write(fdout, &c, 1);
			}
		} else {
			close(fdout);
			return(0);
		}
	}

	return(0);
}

void int_handler(int sig)
{
	UNUSED(sig);

	close(fdin);
	close(fdout);
	exit(0);
}
