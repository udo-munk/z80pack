/*
 * Write the CP/M system files to system tracks of drive A
 *
 * Copyright (C) 1988-2016 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * History:
 * 29-APR-1988 Development on TARGON/35 with AT&T Unix System V.3
 * 11-MAR-1993 comments in english and ported to COHERENT 4.0
 * 02-OCT-2006 modified to compile on modern POSIX OS's
 * 10-JAN-2014 lseek POSIX conformance
 * 03-APR-2016 disk drive name drivea.dsk
 * 27-APR-2024 improve error handling, use simple binary format
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define DISK "../disks/drivea.dsk"

/*
 *	This program writes the CP/M 2.2 OS from the following files
 *	onto the system tracks of the boot disk (DISK):
 *
 *	boot loader	boot.bin
 *	CCP		cpm.bin
 *	BDOS		cpm.bin
 *	BIOS		bios.bin
 */
int main(void)
{
	unsigned char sector[128];
	register int i;
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

	/* open CP/M system file (cpm.bin) for reading */
	if ((fd = open("cpm.bin", O_RDONLY)) == -1) {
		perror("cpm.bin");
		exit(EXIT_FAILURE);
	}
	/* position to CCP in cpm.bin, needed if created with e.g. SAVE */
	if ((o = lseek(fd, 17 * 128L, SEEK_SET)) != 17 * 128L) {
		fprintf(stderr, "cpm.bin: %s\n",
			o == -1 ? strerror(errno) : "short file");
		exit(EXIT_FAILURE);
	}
	/* read CCP and BDOS from cpm.bin and write them to disk in drive A */
	for (i = 0; i < 44; i++) {
		if ((n = read(fd, (char *) sector, 128)) != 128) {
			fprintf(stderr, "cpm.bin: %s\n",
				n == -1 ? strerror(errno) : "short read");
			exit(EXIT_FAILURE);
		}
		if ((n = write(drivea, (char *) sector, 128)) != 128) {
			fprintf(stderr, DISK ": %s\n",
				n == -1 ? strerror(errno) : "short write");
			exit(EXIT_FAILURE);
		}
	}
	close(fd);

	/* open BIOS (bios.bin) for reading */
	if ((fd = open("bios.bin", O_RDONLY)) == -1) {
		perror("bios.bin");
		exit(EXIT_FAILURE);
	}
	/* read BIOS from bios.bin and write it to disk in drive A */
	i = 0;
	while ((n = read(fd, (char *) sector, 128)) == 128) {
		if (i == 7) {
			fputs(DISK ": out of space (7 sectors written)\n",
			      stderr);
			exit(EXIT_FAILURE);
		}
		if ((n = write(drivea, (char *) sector, 128)) != 128) {
			fprintf(stderr, DISK ": %s\n",
				n == -1 ? strerror(errno) : "short write");
			exit(EXIT_FAILURE);
		}
		i++;
	}
	if (n == -1) {
		perror("bios.bin");
		exit(EXIT_FAILURE);
	} else if (n > 0) {
		if (i == 7) {
			fputs(DISK ": out of space (7 sectors written)\n",
			      stderr);
			exit(EXIT_FAILURE);
		}
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
