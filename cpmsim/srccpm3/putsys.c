/*
 * Write the CP/M 3 systemfiles to system tracks of drive A
 *
 * Copyright (C) 1988-2016 by Udo Munk
 *
 * History:
 * 29-APR-88 Development on TARGON/35 with AT&T Unix System V.3
 * 11-MAR-93 comments in english and ported to COHERENT 4.0
 * 02-OCT-06 modified to compile on modern POSIX OS's
 * 15-SEP-07 also write ccp to system tracks
 * 10-JAN-14 lseek POSIX conformance
 * 03-APR-16 disk drive name drivea.dsk
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

/*
 *	This program writes the CP/M 3 OS from the following files
 *	onto the system tracks of the boot disk (drivea.dsk):
 *
 *	boot loader	boot.bin
 *	cpmldr		cpmldr.bin
 *	ccp		ccp.bin
 */
int main(void)
{
	unsigned char sector[128];
	int fd, drivea;

	/* open drive A for writing */
	if ((drivea = open("../disks/drivea.dsk", O_WRONLY)) == -1) {
		perror("file ../disks/drivea.dsk");
		exit(EXIT_FAILURE);
	}
	/* open boot loader (boot.bin) for reading */
	if ((fd = open("boot.bin", O_RDONLY)) == -1) {
		perror("file boot.bin");
		exit(EXIT_FAILURE);
	}
	/* read boot loader */
	memset((char *) sector, 0, 128);
	read(fd, (char *) sector, 128);
	close(fd);
	/* and write it to disk in drive A */
	write(drivea, (char *) sector, 128);
	/* open CP/M 3 cpmldr file (cpmldr.bin) for reading */
	if ((fd = open("cpmldr.bin", O_RDONLY)) == -1) {
		perror("file cpmldr.bin");
		exit(EXIT_FAILURE);
	}
	/* read from cpmldr.bin and write to disk in drive A */
	while (read(fd, (char *) sector, 128) == 128)
		write(drivea, (char *) sector, 128);
	write(drivea, (char *) sector, 128);
	close(fd);
	/* seek to track 1, sector 1 of disk */
	lseek(drivea, 128 * 26, SEEK_SET);
	/* open CP/M 3 ccp file (ccp.bin) for reading */
	if ((fd = open("ccp.bin", O_RDONLY)) == -1) {
		perror("file ccp.bin");
		exit(EXIT_FAILURE);
	}
	/* read from ccp.bin and write to disk in drive A */
	while (read(fd, (char *) sector, 128) == 128)
		write(drivea, (char *) sector, 128);
	write(drivea, (char *) sector, 128);
	close(fd);
	close(drivea);
	return (EXIT_SUCCESS);
}
