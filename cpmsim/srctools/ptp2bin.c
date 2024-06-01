/*
 * converts binary paper tape files to raw binary by removing
 * the leading 0's
 *
 * Copyright (C) 2015 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * History:
 * 04-APR-2015 first version
 * 27-APR-2024 improve error handling
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	int fdin, fdout;
	int flag = 0;
	ssize_t n;
	char c;
	char *pn = basename(argv[0]);

	if (argc != 3) {
		printf("usage: %s infile outfile\n", pn);
		exit(EXIT_FAILURE);
	}

	if ((fdin = open(argv[1], O_RDONLY)) == -1) {
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}

	if ((fdout = open(argv[2], O_WRONLY | O_CREAT, 0644)) == -1) {
		perror(argv[2]);
		exit(EXIT_FAILURE);
	}

	while ((n = read(fdin, &c, 1)) == 1) {
		if (flag == 0) {
			if (c == 0)
				continue;
			else
				flag++;
		}
		if ((n = write(fdout, &c, 1)) != 1) {
			fprintf(stderr, "%s: %s\n", argv[2],
				n == -1 ? strerror(errno) : "short write");
			exit(EXIT_FAILURE);
		}
	}
	if (n == -1) {
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}

	close(fdin);
	close(fdout);

	return (EXIT_SUCCESS);
}
