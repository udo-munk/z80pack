/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2017 by Udo Munk
 *
 * Emulation of a Tarbell SD 1011D S100 board
 *
 * History:
 * 13-MAR-2014 first fully working version
 * 15-MAR-2014 some improvements for CP/M 1.3 & 1.4
 * 17-MAR-2014 close(fd) was missing in write sector lseek error case
 *    AUG-2014 some improvements
 * 22-JAN-2015 fixed buggy ID field
 * 11-FEB-2015 implemented write track
 * 08-MAR-2016 support user path for disk images
 * 13-MAY-2016 find disk images at -d <path>, ./disks and DISKDIR
 * 22-JUL-2016 added support for read only disks
 * 13-JUN-2017 added bootstrap ROM and reset function
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sim.h"
#include "simglb.h"

/* internal state of the fdc */
#define FDC_IDLE	0	/* idle state */
#define FDC_READ	1	/* reading sector */
#define FDC_WRITE	2	/* writing sector */
#define FDC_READADR	3	/* read address */
#define FDC_WRTTRK	4	/* write (format) track */

/* 8" standard disks */
#define SEC_SZ		128
#define SPT		26
#define TRK		77

static BYTE fdc_stat;		/* status register */
static BYTE fdc_track;		/* track register */
static BYTE fdc_sec;		/* sector register */
static int disk;		/* current disk # */
static int state;		/* fdc state */
static char fn[4096];		/* path/filename for disk image */
static int fd;			/* fd for disk file i/o */
static int dcnt;		/* data counter read/write */
static BYTE buf[SEC_SZ];	/* buffer for one sector */

/* these are our disk drives */
static char *disks[4] = {
	"drivea.dsk",
	"driveb.dsk",
	"drivec.dsk",
	"drived.dsk"
};

/* bootstrap ROM */
BYTE tarbell_rom[32] = {
	0xdb, 0xfc, 0xaf, 0x6f, 0x67, 0x3c, 0xd3, 0xfa,
        0x3e, 0x8c, 0xd3, 0xf8, 0xdb, 0xfc, 0xb7, 0xf2,
	0x19, 0x00, 0xdb, 0xfb, 0x77, 0x23, 0xc3, 0x0c,
	0x00, 0xdb, 0xf8, 0xb7, 0xca, 0x7d, 0x00, 0x76 };

int tarbell_rom_enabled = 0;	/* ROM not enabled by default */
int tarbell_rom_active = 1;	/* ROM is active at power on */

/*
 * find and set path for disk images
 */
void dsk_path(void) {
	struct stat sbuf;

	/* if option -d is used disks are there */
	if (diskdir != NULL) {
		strcpy(fn, diskd);
	} else {
		/* if not first try ./disks */
		if ((stat("./disks", &sbuf) == 0) && S_ISDIR(sbuf.st_mode)) {
			strcpy(fn, "./disks");
		/* nope, then DISKSDIR as set in Makefile */
		} else {
			strcpy(fn, DISKSDIR);
		}
	}
}

/*
 * FDC status port
 */
BYTE tarbell_stat_in(void)
{
	return(fdc_stat);
}

/*
 * FDC command port
 */
void tarbell_cmd_out(BYTE data)
{
	if ((data & 0xf0) == 0) {		/* restore (seek track 0) */
		fdc_track = 0;			/*          ^ ^ */
		fdc_stat = 4;			/* positioned to track 0 */

	} else if ((data & 0xf0) == 0x10) {	/* seek */
		//printf("tarbell: seek to track %d\r\n", fdc_track);
		if (fdc_track <= TRK)
			fdc_stat = 0;
		else
			fdc_stat = 0x10;	/* seek error */

	} else if ((data & 0xe0) == 0x40) {	/* step in */
		//printf("tarbell: step in\r\n");
		if (data & 0x10)
			fdc_track++;
		if (fdc_track <= TRK)
			fdc_stat = 0;
		else
			fdc_stat = 0x10;	/* seek error */

	} else if ((data & 0xe0) == 0x50) {	/* step out */
		//printf("tarbell: step out\r\n");
		if (data & 0x10)
			fdc_track--;
		if (fdc_track <= TRK)
			fdc_stat = 0;
		else
			fdc_stat = 0x10;	/* seek error */

	} else if ((data & 0xf0) == 0x80) {	/* read single sector */
		state = FDC_READ;
		dcnt = 0;
		fdc_stat = 0;

	} else if ((data & 0xf0) == 0x90) {	/* read multiple sector */
		printf("tarbell: read multiple sector not implemented\r\n");
		fdc_stat = 0x10;		/* record not found */

	} else if ((data & 0xf0) == 0xa0) {	/* write single sector */
		state = FDC_WRITE;
		dcnt = 0;
		fdc_stat = 0;

	} else if ((data & 0xf0) == 0xb0) {	/* write multiple sector */
		printf("tarbell: write multiple sector not implemented\r\n");
		fdc_stat = 0x10;		/* record not found */

	} else if (data == 0xc4) {		/* read address */
		state = FDC_READADR;
		dcnt = 0;
		fdc_stat = 0;

	} else if ((data & 0xf0) == 0xe0) {	/* read track */
		printf("tarbell: read track not implemented\r\n");
		fdc_stat = 0x10;		/* record not found */

	} else if ((data & 0xf0) == 0xf0) {	/* write track */
		state = FDC_WRTTRK;
		dcnt = 0;
		fdc_stat = 0;

	} else if ((data & 0xf0) == 0xd0) {	/* force interrupt */
		//printf("tarbell: interrupt\r\n");
		state = FDC_IDLE;		/* abort any command */
		fdc_stat = 0;

	} else {
		printf("tarbell: unknown command, %02x\r\n", data);
		fdc_stat = 8;			/* CRC error */
	}
}

/*
 * FDC track input port
 */
BYTE tarbell_track_in(void)
{
	return(fdc_track);
}

/*
 * FDC track output port
 */
void tarbell_track_out(BYTE data)
{
	fdc_track = data;
}

/*
 * FDC sector input port
 */
BYTE tarbell_sec_in(void)
{
	return(fdc_sec);
}

/*
 * FDC sector output port
 */
void tarbell_sec_out(BYTE data)
{
	fdc_sec = data;
}

/*
 * FDC data input port
 */
BYTE tarbell_data_in(void)
{
	long pos;		/* seek position */

	switch (state) {
	case FDC_READ:		/* read data from disk sector */

		/* first byte? */
		if (dcnt == 0) {

			//printf("tarbell read: t = %d, s = %d\r\n",
			//       fdc_track, fdc_sec);

			/* check track/sector */
			if ((fdc_track >= TRK) || (fdc_sec > SPT)) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = 0x10;	/* record not found */
				return((BYTE) 0);
			}

			/* check disk drive */
			if ((disk < 0) || (disk > 3)) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = 0x80;	/* not ready */
				//printf("tarbell read: disk = %d\r\n", disk);
				return((BYTE) 0);
			}

			/* try to open disk image */
			dsk_path();
			strcat(fn, "/");
			strcat(fn, disks[disk]);
			if ((fd = open(fn, O_RDONLY)) == -1) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = 0x80;	/* not ready */
				//printf("tarbell read: open error disk %d\r\n",
				//       disk);
				return((BYTE) 0);
			}

			/* seek to sector */
			pos = (fdc_track * SPT + fdc_sec - 1) * SEC_SZ;
			if (lseek(fd, pos, SEEK_SET) == -1L) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = 0x10;	/* record not found */
				//printf("tarbell read: seek error\r\n");
				close(fd);
				return((BYTE) 0);
			}

			/* read the sector */
			if (read(fd, &buf[0], SEC_SZ) != SEC_SZ) {
				state = FDC_IDLE;	/* abort read command */
				fdc_stat = 0x10;	/* record not found */
				//printf("tarbell read: read error\r\n");
				close(fd);
				return((BYTE) 0);
			}
			close(fd);
		}

		/* last byte? */
		if (dcnt == SEC_SZ - 1) {
			state = FDC_IDLE;		/* reset DRQ */
			fdc_stat = 0;
		}

		/* return byte from buffer and increment counter */
		return(buf[dcnt++]);
		break;

	case FDC_READADR:	/* read disk address */

		/* first byte? */
		if (dcnt == 0) {
			buf[0] = fdc_track;	/* build address field */
			buf[1] = 0;
			buf[2] = fdc_sec;
			buf[3] = 0;
			buf[4] = 0;
			buf[5] = 0;
		}

		/* last byte? */
		if (dcnt == 5) {
			state = FDC_IDLE;	/* reset DRQ */
			fdc_stat = 0;
		}

		/* return byte from buffer and increment counter */
		return(buf[dcnt++]);
		break;

	default:
		return((BYTE) 0);
		break;
	}
}

/*
 * FDC data output port
 */
void tarbell_data_out(BYTE data)
{
	long pos;			/* seek position */
	static int wrtstat;		/* state while formatting track */
	static int bcnt;		/* byte counter for sector data */
	static int secs;		/* # of sectors written so far */

	switch (state) {
	case FDC_WRITE:			/* write data to disk sector */
		if (dcnt == 0) {

			/* check track/sector */
			if ((fdc_track >= TRK) || (fdc_sec > SPT)) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = 0x10;	/* record not found */
				return;
			}

			/* check disk drive */
			if ((disk < 0) || (disk > 3)) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = 0x80;	/* not ready */
				//printf("tarbell write: disk = %d\r\n", disk);
				return;
			}

			/* try to open disk image */
			dsk_path();
			strcat(fn, "/");
			strcat(fn, disks[disk]);
			if ((fd = open(fn, O_RDWR)) == -1) {
				if ((fd = open(fn, O_RDONLY)) != -1) {
					close(fd);
					fdc_stat = 0x40; /* read only */
				} else {
					fdc_stat = 0x80; /* not ready */
				}
				state = FDC_IDLE;	/* abort command */
				//printf("tarbell write: open error disk %d\r\n", disk);
				return;
			}

			/* seek to sector */
			pos = (fdc_track * SPT + fdc_sec - 1) * SEC_SZ;
			if (lseek(fd, pos, SEEK_SET) == -1L) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = 0x10;	/* record not found */
				//printf("tarbell write: seek error\r\n");
				close(fd);
				return;
			}
		}

		/* write data bytes into sector buffer */
		buf[dcnt++] = data;

		/* last byte? */
		if (dcnt == SEC_SZ) {
			state = FDC_IDLE;		/* reset DRQ */
			if (write(fd, &buf[0], SEC_SZ) == SEC_SZ)
				fdc_stat = 0;
			else
				fdc_stat = 0x10;	/* record not found */
			close(fd);
		}
		break;

	case FDC_WRTTRK:		/* write (format) TRACK */
		if (dcnt == 0) {
			/* unlink disk image */
			dsk_path();
			strcat(fn, "/");
			strcat(fn, disks[disk]);
			if (fdc_track == 0)
				unlink(fn);
			/* try to create new disk image */
			if ((fd = open(fn, O_RDWR|O_CREAT, 0644)) == -1) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = 0x80;	/* not ready */
				return;
			}
			/* seek to track */
			pos = fdc_track * SPT  * SEC_SZ;
			if (lseek(fd, pos, SEEK_SET) == -1L) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = 0x10;	/* record not found */
				close(fd);
				return;
			}
			/* now wait for sector data */
			wrtstat = 1;
			secs = 0;
		}
		dcnt++;
		/* wait for sector data address mark */
		if (wrtstat == 1) {
			if (data == 0xfb) {
				wrtstat = 2;
				bcnt = 0;
			}
			return;
		}
		/* collect bytes in buffer and write if sector complete */
		if (wrtstat == 2) {
			if (data != 0xf7) {
				buf[bcnt++] = data;
				return;
			} else {
				secs++;
				if (write(fd, buf, bcnt) == bcnt)
					fdc_stat = 0;
				else
					fdc_stat = 0x10; /* record not found */
				wrtstat = 1;
			}
		}
		/* all sectors of track written? */
		if (secs == SPT) {
			state = FDC_IDLE;
			close(fd);
		}
		break;

	default:			/* track # for seek */
		fdc_track = data;
		break;
	}
}

/*
 * FDC wait input port
 */
BYTE tarbell_wait_in(void)
{
	if (state == FDC_IDLE)
		return((BYTE) 0);	/* don't wait for drive mechanics */
	else
		return((BYTE) 0x80);	/* but wait on DRQ */
}

/*
 * FDC extended command output port
 */
void tarbell_ext_out(BYTE data)
{
	disk = ~data >> 4 & 3;		/* get disk # */
}

/*
 * Reset FDC
 */
void tarbell_reset(void)
{
	fdc_stat = fdc_track = fdc_sec = disk = state = dcnt = 0;
	tarbell_rom_active = 1;
}
