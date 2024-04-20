/*
 * make disk image files for cpmsim
 *
 * Copyright (C) 1988-2016 by Udo Munk
 *
 * History:
 * 29-APR-88 Development on TARGON/35 with AT&T Unix System V.3
 * 11-MAR-93 comments in english
 * 01-OCT-06 modified to compile on modern POSIX OS's
 * 18-NOV-06 added a second harddisk
 * 01-OCT-07 added a huge 512MB harddisk
 * 11-NOV-07 abort if file already exists
 * 09-FEB-14 name changed from format to mkdskimg
 * 14-JAN-16 make disk file in directory drives if exists, in cwd otherwise
 * 14-MAR-16 renamed the used disk images to drivex.dsk
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define TRACK   	77
#define SECTOR  	26
#define HDTRACK		255
#define HDSECTOR	128
#define HD2TRACK	256
#define HD2SECTOR	16384

/*
 *	This program creates image files for the following disk formats:
 *
 *		drive A:	8" IBM SS,SD
 *		drive B:	8" IBM SS,SD
 *		drive C:	8" IBM SS,SD
 *		drive D:	8" IBM SS,SD
 *		drive I:	4MB harddisk
 *		drive J:	4MB harddisk
 *		drive P:	512MB harddisk
 */
int main(int argc, char *argv[])
{
	register int i;
	int fd;
	char drive;
	struct stat sb;
	static unsigned char sector[128];
	static char fn[64];
	static char ddir[] = "disks";
	static char dn[] = "drive?.dsk";
	static char usage[] = "usage: mkdskimg a | b | c | d | i | j | p";

	if (argc != 2) {
		puts(usage);
		exit(EXIT_FAILURE);
	}
	i = *argv[1];
	if (argc != 2 ||
	    (i != 'a' && i != 'b' && i != 'c' && i != 'd' && i != 'i'
	     && i != 'j' && i != 'p')) {
		puts(usage);
		exit(EXIT_FAILURE);
	}
	dn[5] = drive = (char) i;
	if (stat(ddir, &sb) == 0 && S_ISDIR(sb.st_mode)) {
		strcpy(fn, ddir);
		strcat(fn, "/");
		strcat(fn, dn);
	} else {
		strcpy(fn, dn);
	}
	memset((char *) sector, 0xe5, 128);
	if ((fd = open(fn, O_RDONLY)) != -1) {
		printf("disk file %s exists, aborting\n", fn);
		exit(EXIT_FAILURE);
	}
	if ((fd = creat(fn, 0644)) == -1) {
		perror("disk file");
		exit(EXIT_FAILURE);
	}
	if (drive <= 'd') {
		for (i = 0; i < TRACK * SECTOR; i++)
			write(fd, (char *) sector, 128);
	} else if (drive == 'i' || drive == 'j') {
		for (i = 0; i < HDTRACK * HDSECTOR; i++)
			write(fd, (char *) sector, 128);
	} else if (drive == 'p') {
		for (i = 0; i < HD2TRACK * HD2SECTOR; i++)
			write(fd, (char *) sector, 128);
	}
	close(fd);
	return(EXIT_SUCCESS);
}
