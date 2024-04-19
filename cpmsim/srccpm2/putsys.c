/*
 * Write the CP/M systemfiles to system tracks of drive A
 *
 * Copyright (C) 1988-2016 by Udo Munk
 *
 * History:
 * 29-APR-88 Development on TARGON/35 with AT&T Unix System V.3
 * 11-MAR-93 comments in english and ported to COHERENT 4.0
 * 02-OCT-06 modified to compile on modern POSIX OS's
 * 10-JAN-14 lseek POSIX conformance
 * 03-APR-16 disk drive name drivea.dsk
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

/*
 *	This program writes the CP/M 2.2 OS from the following files
 *	onto the system tracks of the boot disk (drivea.dsk):
 *
 *	boot loader	boot.bin	(Mostek binary format)
 *	CCP		cpm.bin		(binary format)
 *	BDOS		cpm.bin		(binary format)
 *	BIOS		bios.bin	(Mostek binary format)
 */
int main(void)
{
	unsigned char header[3];
	unsigned char sector[128];
	register int i;
	int fd, drivea, readn;

	/* open drive A for writing */
	if ((drivea = open("../disks/drivea.dsk", O_WRONLY)) == -1) {
		perror("file ../disks/drivea.dsk");
		exit(EXIT_FAILURE);
	}
	/* open boot loader (boot.bin) for reading */
	if ((fd = open("boot.bin", O_RDONLY)) == -1) {
		perror("file boot.bin");
		close(drivea);
		exit(EXIT_FAILURE);
	}
	/* read and check 3 byte header */
	if (read(fd, (char *) header, 3) != 3) {
		perror("file boot.bin");
		close(fd);
		close(drivea);
		exit(EXIT_FAILURE);
	}
	if (header[0] != 0xff || header[1] != 0 || header[2] != 0) {
		puts("start address of boot.bin <> 0");
		close(fd);
		close(drivea);
		exit(EXIT_SUCCESS);
	}
	/* read boot loader */
	memset((char *) sector, 0, 128);
	read(fd, (char *) sector, 128);
	close(fd);
	/* and write it to disk in drive A */
	write(drivea, (char *) sector, 128);
	/* open CP/M system file (cpm.bin) for reading */
	if ((fd = open("cpm.bin", O_RDONLY)) == -1) {
		perror("file cpm.bin");
		close(drivea);
		exit(EXIT_FAILURE);
	}
	/* position to CCP in cpm.bin, needed if created with SAVE or similar */
	lseek(fd, (long) 17 * 128, SEEK_SET);
	/* read CCP and BDOS from cpm.bin and write them to disk in drive A */
	for (i = 0; i < 44; i++) {
		if (read(fd, (char *) sector, 128) != 128) {
			perror("file cpm.bin");
			close(fd);
			close(drivea);
			exit(EXIT_FAILURE);
		}
		write(drivea, (char *) sector, 128);
	}
	close(fd);
	/* open BIOS (bios.bin) for reading */
	if ((fd = open("bios.bin", O_RDONLY)) == -1) {
		perror("file bios.bin");
		close(drivea);
		exit(EXIT_FAILURE);
	}
	/* read and check 3 byte header */
	if (read(fd, (char *) header, 3) != 3) {
		perror("file bios.bin");
		close(fd);
		close(drivea);
		exit(EXIT_FAILURE);
	}
	if (header[0] != 0xff) {
		puts("unknown format of bios.bin");
		close(fd);
		close(drivea);
		exit(EXIT_SUCCESS);
	}
	/* read BIOS from bios.bin and write it to disk in drive A */
	i = 0;
	while ((readn = read(fd, (char *) sector, 128)) == 128) {
		write(drivea, (char *) sector, 128);
		i++;
		if (i == 6) {
			puts("6 sectors written, can't write any more!");
			goto stop;
		}
	}
	if (readn > 0) {
		write(drivea, (char *) sector, 128);
	}
stop:
	close(fd);
	close(drivea);
	return(EXIT_SUCCESS);
}
