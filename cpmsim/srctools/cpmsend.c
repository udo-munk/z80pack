/*
 * Sends a file through named pipe "auxin" to the CP/M simulation
 *
 * Copyright (C) 1988-2017 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * History:
 * 05-OCT-1988 Development on TARGON/35 with AT&T Unix System V.3
 * 11-MAR-1993 comments in english and ported to COHERENT 4.0
 * 01-OCT-2006 modified to compile on modern POSIX OS's
 * 09-MAR-2016 moved pipes to /tmp/.z80pack
 * 20-MAR-2017 renamed pipe
 * 27-APR-2024 improve error handling
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

void sendbuf(ssize_t);

char buf[BUFSIZ];
char cr = '\r';
int fdout, fdin;

int main(int argc, char *argv[])
{
	ssize_t n;

	if (argc != 2) {
		puts("usage: cpmsend filename &");
		exit(EXIT_FAILURE);
	}
	if ((fdin = open(argv[1], O_RDONLY)) == -1) {
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}
	if ((fdout = open("/tmp/.z80pack/cpmsim.auxin", O_WRONLY)) == -1) {
		perror("auxin pipe");
		exit(EXIT_FAILURE);
	}
	while ((n = read(fdin, buf, BUFSIZ)) == BUFSIZ)
		sendbuf(BUFSIZ);
	if (n == -1) {
		perror(argv[1]);
		exit(EXIT_FAILURE);
	} else if (n > 0)
		sendbuf(n);
	close(fdin);
	close(fdout);
	return EXIT_SUCCESS;
}

void sendbuf(ssize_t size)
{
	register char *s = buf;
	ssize_t n;

	while (s - buf < size) {
		if (*s == '\n')
			if ((n = write(fdout, (char *) &cr, 1)) != 1) {
				fprintf(stderr, "auxin pipe: %s\n",
					n == -1 ? strerror(errno)
						: "short write");
				exit(EXIT_FAILURE);
			}
		if ((n = write(fdout, s++, 1)) != 1) {
			fprintf(stderr, "auxin pipe: %s\n",
				n == -1 ? strerror(errno) : "short write");
			exit(EXIT_FAILURE);
		}
	}
}
