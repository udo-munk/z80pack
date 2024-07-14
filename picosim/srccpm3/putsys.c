/*
 * Write the CP/M 3 system files to system tracks of drive A
 *
 * Copyright (C) 1988-2016 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * History:
 * 29-APR-1988 Development on TARGON/35 with AT&T Unix System V.3
 * 11-MAR-1993 comments in english and ported to COHERENT 4.0
 * 02-OCT-2006 modified to compile on modern POSIX OS's
 * 15-SEP-2007 also write ccp to system tracks
 * 10-JAN-2014 lseek POSIX conformance
 * 03-APR-2016 disk drive name drivea.dsk
 * 27-APR-2024 improve error handling
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

/*#define DISK "../disks/drivea.dsk"*/
#define DISK "../disks/cpm3-1.dsk"

/*
 *	This program writes the CP/M 3 OS from the following files
 *	onto the system tracks of the boot disk (DISK):
 *
 *	boot loader	boot.bin
 *	cpmldr		cpmldr.bin
 *	ccp		ccp.bin
 */
int main(void)
{
	unsigned char sector[128];
	int fd, drivea;
	ssize_t n;
	off_t o;

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

	/* open CP/M 3 cpmldr file (cpmldr.bin) for reading */
	if ((fd = open("cpmldr.bin", O_RDONLY)) == -1) {
		perror("cpmldr.bin");
		exit(EXIT_FAILURE);
	}
	/* read from cpmldr.bin and write to disk in drive A */
	while ((n = read(fd, (char *) sector, 128)) == 128)
		if ((n = write(drivea, (char *) sector, 128)) != 128) {
			fprintf(stderr, DISK ": %s\n",
				n == -1 ? strerror(errno) : "short write");
			exit(EXIT_FAILURE);
		}
	if (n == -1) {
		perror("cpmldr.bin");
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

	/* seek to track 1, sector 1 of disk */
	if ((o = lseek(drivea, 26 * 128L, SEEK_SET)) != 26 * 128L) {
		fprintf(stderr, DISK ": %s\n",
			o == -1 ? strerror(errno) : "short file");
		exit(EXIT_FAILURE);
	}

	/* open CP/M 3 ccp file (ccp.bin) for reading */
	if ((fd = open("ccp.bin", O_RDONLY)) == -1) {
		perror("ccp.bin");
		exit(EXIT_FAILURE);
	}
	/* read from ccp.bin and write to disk in drive A */
	while ((n = read(fd, (char *) sector, 128)) == 128)
		if ((n = write(drivea, (char *) sector, 128)) != 128) {
			fprintf(stderr, DISK ": %s\n",
				n == -1 ? strerror(errno) : "short write");
			exit(EXIT_FAILURE);
		}
	if (n == -1) {
		perror("ccp.bin");
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
