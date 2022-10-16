/*
 * Write the MP/M 2 systemfiles to system tracks of drive A
 *
 * Copyright (C) 2006-2016 by Udo Munk
 *
 * History:
 * 08-DEC-06 cloned from the version for CP/M 3
 * 03-APR-16 disk drive name drivea.dsk
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

/*
 *	This program writes the MP/M 2 OS from the following files
 *	onto the system tracks of the boot disk (drivea.dsk):
 *
 *	boot loader	boot.bin
 *	mpmldr		mpmldr.bin
 */
int main(void)
{
	unsigned char sector[128];
	int fd, drivea, readn;

	/* open drive A for writing */
	if ((drivea = open("../disks/drivea.dsk", O_WRONLY)) == -1) {
		perror("file ../disks/drivea.dsk");
		exit(1);
	}
	/* open boot loader (boot.bin) for reading */
	if ((fd = open("boot.bin", O_RDONLY)) == -1) {
		perror("file boot.bin");
		exit(1);
	}
	/* read boot loader */
	memset((char *) sector, 0, 128);
	read(fd, (char *) sector, 128);
	close(fd);
	/* and write it to disk in drive A */
	write(drivea, (char *) sector, 128);
	/* open MP/M 2 mpmldr file (mpmldr.bin) for reading */
	if ((fd = open("mpmldr.bin", O_RDONLY)) == -1) {
		perror("file mpmldr.bin");
		exit(1);
	}
	/* read from mpmldr.bin and write to disk in drive A */
	while ((readn = read(fd, (char *) sector, 128)) == 128)
		write(drivea, (char *) sector, 128);
	write(drivea, (char *) sector, 128);
	close(fd);
	close(drivea);
	return(0);
}
