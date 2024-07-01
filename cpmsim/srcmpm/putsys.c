/*
 * Write the MP/M 2 system files to system tracks of drive A
 *
 * Copyright (C) 2006-2016 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * History:
 * 08-DEC-2006 cloned from the version for CP/M 3
 * 03-APR-2016 disk drive name drivea.dsk
 * 27-APR-2024 improve error handling
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define DISK "../disks/drivea.dsk"
/*
 *	This program writes the MP/M 2 OS from the following files
 *	onto the system tracks of the boot disk (DISK):
 *
 *	boot loader	boot.bin
 *	mpmldr		mpmldr.bin
 */
int main(void)
{
	unsigned char sector[128];
	int fd, drivea;
	ssize_t n;

	/* open drive A for writing */
	if ((drivea = open(DISK, O_WRONLY)) == -1) {
		perror(DISK);
		exit(EXIT_FAILURE);
	}

	/* open boot loader (boot.bin) for reading */
	if ((fd = open("boot.bin", O_RDONLY)) == -1) {
		perror("boot.bin");
		exit(EXIT_FAILURE);
	}
	/* read boot loader */
	memset((char *) sector, 0, 128);
	if (read(fd, (char *) sector, 128) == -1) {
		perror("boot.bin");
		exit(EXIT_FAILURE);
	}
	/* and write it to disk in drive A */
	if ((n = write(drivea, (char *) sector, 128)) != 128) {
		fprintf(stderr, DISK ": %s\n",
			n == -1 ? strerror(errno) : "short write");
		exit(EXIT_FAILURE);
	}
	close(fd);

	/* open MP/M 2 mpmldr file (mpmldr.bin) for reading */
	if ((fd = open("mpmldr.bin", O_RDONLY)) == -1) {
		perror("mpmldr.bin");
		exit(EXIT_FAILURE);
	}
	/* read from mpmldr.bin and write to disk in drive A */
	while ((n = read(fd, (char *) sector, 128)) == 128)
		if ((n = write(drivea, (char *) sector, 128)) != 128) {
			fprintf(stderr, DISK ": %s\n",
				n == -1 ? strerror(errno) : "short write");
			exit(EXIT_FAILURE);
		}
	if (n == -1) {
		perror("mpmldr.bin");
		exit(EXIT_FAILURE);
	} else if (n > 0) {
		memset((char *) &sector[n], 0, 128 - n);
		if ((n = write(drivea, (char *) sector, 128)) != 128) {
			fprintf(stderr, DISK ": %s\n",
				n == -1 ? strerror(errno) : "short write");
			exit(EXIT_FAILURE);
		}
	}
	close(fd);

	close(drivea);
	return EXIT_SUCCESS;
}
