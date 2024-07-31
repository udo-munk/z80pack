/*
 * Write the UCSD pSystem IV boot and BIOS code to system tracks of drive A
 *
 * Copyright (C) 2008-2016 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * History:
 * 12-AUG-2008 initial version
 * 03-APR-2016 disk drive name drivea.dsk
 * 27-APR-2024 improve error handling
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

//#define DISK "../disks/drivea.dsk"
#define DISK "../disks/ucsd1.dsk"

/*
 *	This program writes the UCSD boot code from the following files
 *	onto the system tracks of the boot disk (DISK):
 *
 *	boot loader	boot.bin
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

	/* seek to sector 19 on drive A */
	if ((o = lseek(drivea, 18 * 128L, SEEK_SET)) != 18 * 128L) {
		fprintf(stderr, DISK ": %s\n",
			o == -1 ? strerror(errno) : "short file");
		exit(EXIT_FAILURE);
	}

	/* open BIOS (bios.bin) for reading */
	if ((fd = open("bios.bin", O_RDONLY)) == -1) {
		perror("bios.bin");
		exit(EXIT_FAILURE);
	}
	/* read BIOS from bios.bin and write it to disk in drive A */
	i = 0;
	while ((n = read(fd, (char *) sector, 128)) == 128) {
		if ((n = write(drivea, (char *) sector, 128)) != 128) {
			fprintf(stderr, DISK ": %s\n",
				n == -1 ? strerror(errno) : "short write");
			exit(EXIT_FAILURE);
		}
		i++;
		if (i == 8) {
			fputs(DISK ": out of space (8 sectors written)",
			      stderr);
			exit(EXIT_FAILURE);
		}
	}
	if (n == -1) {
		perror("bios.bin");
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
