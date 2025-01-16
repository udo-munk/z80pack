/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2021 Udo Munk
 * Copyright (C) 2021 David McNaughton
 *
 * Emulation of a Cromemco 4FDC/16FDC S100 board
 *
 * History:
 * 20-DEC-2014 first version
 * 28-DEC-2014 second version with 16FDC, CP/M 2.2 boots
 * 01-JAN-2015 fixed 16FDC, machine now also boots CDOS 2.58 from 8" and 5.25"
 * 01-JAN-2015 fixed frontpanel switch settings, added boot flag to fp switch
 * 12-JAN-2015 implemented dummy write track, so that programs won't hang
 * 22-JAN-2015 fixed buggy ID field, fake index pulses for 300/360 RPM
 * 26-JAN-2015 implemented write track to format SS/SD disks properly
 * 02-FEB-2015 implemented DS/DD disk formats
 * 05-FEB-2015 implemented DS/SD disk formats
 * 06-FEB-2015 implemented write track for all formats
 * 12-FEB-2015 implemented motor control, so that a 16FDC is recognized by CDOS
 * 20-FEB-2015 bug fixes for 1.25 release
 * 08-MAR-2016 support user path for disk images
 * 13-MAY-2016 find disk images at -d <path>, ./disks and DISKDIR
 * 22-JUL-2016 added support for read only disks
 * 22-JUN-2017 added reset function
 * 26-JUL-2017 fixed buggy index pulse implementation
 * 15-AUG-2017 and more fixes for index pulse
 * 22-AUG-2017 provide write protect and track 0 bits for all commands
 * 23-APR-2018 cleanup
 * 20-MAY-2018 improved reset
 * 15-JUL-2018 use logging
 * 09-SEP-2019 added disk format without SD track 0 provided by Alan Cox
 * 24-SEP-2019 restore and seek also affect step direction
 * 17-JUN-2021 allow building machine without frontpanel
 * 29-JUL-2021 add boot config for machine without frontpanel
 * 02-SEP-2021 implement banked ROM
 * 15-MAY-2024 make disk manager standard
 */

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simcfg.h"
#include "simmem.h"

#include "diskmanager.h"
#include "cromemco-fdc.h"

#include "log.h"
static const char *TAG = "16FDC";

/* internal state of the fdc */
#define FDC_IDLE	0	/* idle state */
#define FDC_READ	1	/* reading sector */
#define FDC_WRITE	2	/* writing sector */
#define FDC_READADR	3	/* read address */
#define FDC_WRTTRK	4	/* write (format) track */

#define SEC_SZSD	128	/* sector size SD */
#define SEC_SZDD	512	/* sector size DD */
#define TRK8		77	/* # of tracks 8" */
#define SPT8SD		26	/* # of sectors per track 8" SD */
#define SPT8DD		16	/* # of sectors per track 8" DD */
#define TRK5		40	/* # of tracks 5.25" */
#define SPT5SD		18	/* # of sectors per track 5.25" SD */
#define SPT5DD		10	/* # of sectors per track 5.25" DD */

#define AUTOBOOT	16	/* FDC autoboot jumper, 16|64 = off */

       BYTE fdc_flags = AUTOBOOT; /* FDC flag register, autoboot setting */
static BYTE fdc_cmd;		/* FDC command last send */
static BYTE fdc_stat;		/* FDC status register */
static BYTE fdc_aux;		/* FDC auxiliary status */
static BYTE fdc_track = 0;	/* FDC track register */
static BYTE fdc_sec = 1;	/* FDC sector register */
static int step_dir = -1;	/* last stepping direction */
       enum Disk_type dtype;	/* disk type set */
static enum Disk_density ddens;	/* disk density set */
static int disk = 0;		/* current selected disk # */
static int side = 0;		/* disk side */
static int secsz = SEC_SZSD;	/* current used sector size */
static int state;		/* internal fdc state */
static int dcnt;		/* data counter read/write */
static int mflag;		/* multiple sectors flag */
static char fn[MAX_LFN];	/* path/filename for disk image */
static int fd;			/* fd for disk i/o */
static BYTE buf[SEC_SZDD];	/* buffer for one sector */
       int index_pulse = 0;	/* disk index pulse */
static int autowait;		/* autowait flag */
       int motoron;		/* motor on flag */
       int motortimer;		/* motor on timer */
static int headloaded;		/* head loaded flag */

/* these are our disk drives, 8" SS SD initially */
Diskdef disks[4] = {
	{ NULL, LARGE, SINGLE, ONE, TRK8, SPT8SD, SPT8SD, READWRITE, SINGLE },
	{ NULL, LARGE, SINGLE, ONE, TRK8, SPT8SD, SPT8SD, READWRITE, SINGLE },
	{ NULL, LARGE, SINGLE, ONE, TRK8, SPT8SD, SPT8SD, READWRITE, SINGLE },
	{ NULL, LARGE, SINGLE, ONE, TRK8, SPT8SD, SPT8SD, READWRITE, SINGLE }
};

BYTE fdc_banked_rom[8 << 10]; /* 8K of ROM (from 64FDC) to support RDOS 3 */
int fdc_rom_active = 0;

/*
 * find and set path for disk images
 */
char *dsk_path(void)
{
	struct stat sbuf;

	/* if option -d is used disks are there */
	if (diskdir != NULL) {
		strcpy(fn, diskd);
	} else {
		/* if not first try ./disks */
		if ((stat("./disks", &sbuf) == 0) && S_ISDIR(sbuf.st_mode)) {
			strcpy(fn, "./disks");
		} else {
			/* nope, then DISKSDIR as set in Makefile */
			strcpy(fn, DISKSDIR);
		}
		strncpy(diskd, fn, MAX_LFN);
	}
	return diskd;
}

/*
 * configure drive and disk geometry from image file size
 * and set R/W or R/O mode for the disk
 */
static void config_disk(int fd)
{
	struct stat s;

	fstat(fd, &s);
	if (s.st_mode & S_IWUSR)
		disks[disk].disk_m = READWRITE;
	else
		disks[disk].disk_m = READONLY;

	switch (s.st_size) {
	case 92160:		/* 5.25" SS SD */
		disks[disk].disk_t = SMALL;
		disks[disk].disk_d = SINGLE;
		disks[disk].disk_s = ONE;
		disks[disk].tracks = TRK5;
		disks[disk].sectors = SPT5SD;
		disks[disk].sec0 = SPT5SD;
		disks[disk].disk_d0 = SINGLE;
		break;

	case 184320:		/* 5.25" DS SD */
		disks[disk].disk_t = SMALL;
		disks[disk].disk_d = SINGLE;
		disks[disk].disk_s = TWO;
		disks[disk].tracks = TRK5;
		disks[disk].sectors = SPT5SD;
		disks[disk].sec0 = SPT5SD;
		disks[disk].disk_d0 = SINGLE;
		break;

	case 201984:		/* 5.25" SS DD */
		disks[disk].disk_t = SMALL;
		disks[disk].disk_d = DOUBLE;
		disks[disk].disk_s = ONE;
		disks[disk].tracks = TRK5;
		disks[disk].sectors = SPT5DD;
		disks[disk].sec0 = SPT5SD;
		disks[disk].disk_d0 = SINGLE;
		break;

	case 406784:		/* 5.25" DS DD */
		disks[disk].disk_t = SMALL;
		disks[disk].disk_d = DOUBLE;
		disks[disk].disk_s = TWO;
		disks[disk].tracks = TRK5;
		disks[disk].sectors = SPT5DD;
		disks[disk].sec0 = SPT5SD;
		disks[disk].disk_d0 = SINGLE;
		break;

	case 256256:		/* 8" SS SD */
		disks[disk].disk_t = LARGE;
		disks[disk].disk_d = SINGLE;
		disks[disk].disk_s = ONE;
		disks[disk].tracks = TRK8;
		disks[disk].sectors = SPT8SD;
		disks[disk].sec0 = SPT8SD;
		disks[disk].disk_d0 = SINGLE;
		break;

	case 512512:		/* 8" DS SD */
		disks[disk].disk_t = LARGE;
		disks[disk].disk_d = SINGLE;
		disks[disk].disk_s = TWO;
		disks[disk].tracks = TRK8;
		disks[disk].sectors = SPT8SD;
		disks[disk].sec0 = SPT8SD;
		disks[disk].disk_d0 = SINGLE;
		break;

	case 625920:		/* 8" SS DD */
		disks[disk].disk_t = LARGE;
		disks[disk].disk_d = DOUBLE;
		disks[disk].disk_s = ONE;
		disks[disk].tracks = TRK8;
		disks[disk].sectors = SPT8DD;
		disks[disk].sec0 = SPT8SD;
		disks[disk].disk_d0 = SINGLE;
		break;

	case 1256704:		/* 8" DS DD */
		disks[disk].disk_t = LARGE;
		disks[disk].disk_d = DOUBLE;
		disks[disk].disk_s = TWO;
		disks[disk].tracks = TRK8;
		disks[disk].sectors = SPT8DD;
		disks[disk].sec0 = SPT8SD;
		disks[disk].disk_d0 = SINGLE;
		break;

	case 1261568:		/* 8" DS DD no SD track */
		disks[disk].disk_t = LARGE;
		disks[disk].disk_d = DOUBLE;
		disks[disk].disk_s = TWO;
		disks[disk].tracks = TRK8;
		disks[disk].sectors = SPT8DD;
		disks[disk].sec0 = SPT8DD;
		disks[disk].disk_d0 = DOUBLE;
		break;

	default:
		LOGW(TAG, "disk image %s has unknown format", disks[disk].fn);
		disks[disk].disk_t = UNKNOWN;
		break;
	}
}

/*
 * calculate disk image seek position for track/sector/side
 */
static off_t get_pos(void)
{
	off_t pos = -1L;

	if (disks[disk].disk_s == ONE)
		if (disks[disk].disk_d == SINGLE)
			/* single sided, single density */
			pos = (fdc_track * disks[disk].sectors + fdc_sec - 1)
			      * SEC_SZSD;
		else if (disks[disk].disk_d0 == SINGLE)
			/* single sided, double density, track 0 sd */
			if (fdc_track == 0)
				pos = (fdc_sec - 1) * SEC_SZSD;
			else
				pos = ((fdc_track - 1) * disks[disk].sectors
				       + fdc_sec - 1) * SEC_SZDD
				      + disks[disk].sec0 * SEC_SZSD;
		else	/* single sided, double density */
			pos = (fdc_track * disks[disk].sectors + fdc_sec - 1)
			      * SEC_SZDD;
	else if (disks[disk].disk_d == SINGLE)
		/* double sided, single density */
		pos = ((fdc_track * 2 + side) * disks[disk].sectors
		       + fdc_sec - 1) * SEC_SZSD;
	else if (disks[disk].disk_d0 == SINGLE)
		/* double sided, double density, side 0 track 0 sd */
		if (fdc_track == 0 && side == 0)
			pos = (fdc_sec - 1) * SEC_SZSD;
		else
			pos = ((fdc_track * 2 + side - 1) * disks[disk].sectors
			       + fdc_sec - 1) * SEC_SZDD
			      + disks[disk].sec0 * SEC_SZSD;
	else	/* double sided, double density */
		pos = ((fdc_track * 2 + side) * disks[disk].sectors
		       + fdc_sec - 1) * SEC_SZDD;
	return pos;
}

/*
 *	4FDC		16FDC
 * D7	DRQ		DRQ
 * D6	!BOOT		!BOOT
 * D5	HEADLOAD	HEADLOAD
 * D4	unassigned	!INHIBIT INIT
 * D3	unassigned	MOTOR ON
 * D2	unassigned	MOTOR TIMEOUT
 * D1	unassigned	AUTOWAIT TIMEOUT
 * D0	EOJ		EOJ
 */
BYTE cromemco_fdc_diskflags_in(void)
{
	BYTE ret;

	/* reset EOJ after it was read */
	ret = fdc_flags;
	fdc_flags &= ~1;

	/* process autowait timeout */
	if (!(ret & 1) && !(ret & 128) && (autowait == 1)) {
		ret |= 2;
	}

	/* motor status & timer */
	if (motoron) {
		if (motortimer > 0)
			ret |= 8;
		else {
			motoron = 0;
			ret |= 4;
		}
	}

	/* head loaded */
	if (headloaded)
		ret |= 32;

	return ret;
}

/*
 *	4FDC		16FDC
 * D7	AUTO WAIT	AUTO WAIT
 * D6	unassigned	DOUBLE DENSITY
 * D5	MOTOR ON	MOTOR ON
 * D4	MAXI		MAXI
 * D3	DS4		DS4
 * D2	DS3		DS3
 * D1	DS2		DS2
 * D0	DS1		DS1
 */
void cromemco_fdc_diskctl_out(BYTE data)
{
	/* get autowait */
	autowait = (data & 128) ? 1 : 0;

	/* get selected disk */
	if (data & 8)
		disk = 3;
	else if (data & 4)
		disk = 2;
	else if (data & 2)
		disk = 1;
	else if (data & 1)
		disk = 0;
	else
		disk = 0;

	/* get disk density */
	if (data & 64) {
		ddens = DOUBLE;
		secsz = SEC_SZDD;
	} else {
		ddens = SINGLE;
		secsz = SEC_SZSD;
	}

	/* get disk format */
	if (data & 16) {		/* MAXI means 8" */
		dtype = LARGE;
	} else {			/* else 5.25" */
		dtype = SMALL;
	}

	/* motor on/off */
	if (data & 32) {
		motoron = 1;
		motortimer = 800;
	} else {
		motoron = 0;
		motortimer = 0;
	}
}

/*
 *	read data from FDC
 */
BYTE cromemco_fdc_data_in(void)
{
	off_t pos;		/* seek position */
	int lastsec;		/* last sector of a track */

	switch (state) {
	case FDC_READ:		/* read data from disk sector */
		/* first byte? */
		if (dcnt == 0) {
			motortimer = 800;
			if (disks[disk].fn == NULL) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0x80;	/* not ready */
				return (BYTE) 0;
			}
			/* try to open disk image */
			dsk_path();
			strcat(fn, "/");
			strcat(fn, disks[disk].fn);
			if ((fd = open(fn, O_RDONLY)) == -1) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0x80;	/* not ready */
				return (BYTE) 0;
			}
			/* get drive and disk geometry */
			config_disk(fd);
			if (disks[disk].disk_t == UNKNOWN) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0x10;	/* sector not found */
				close(fd);
				return (BYTE) 0;
			}
			/* check track/sector */
			if ((fdc_track == 0) && (side == 0))
				lastsec = disks[disk].sec0;
			else
				lastsec = disks[disk].sectors;
			if ((fdc_track >= disks[disk].tracks) ||
			    (fdc_sec == 0) ||
			    (fdc_sec > lastsec)) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0x10;	/* sector not found */
				close(fd);
				return (BYTE) 0;
			}
			/* seek to sector */
			pos = get_pos();
			if (lseek(fd, pos, SEEK_SET) == -1L) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0x10;	/* sector not found */
				close(fd);
				return (BYTE) 0;
			}
			/* read the sector */
			if (read(fd, buf, secsz) != secsz) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0x10;	/* sector not found */
				close(fd);
				return (BYTE) 0;
			}
			close(fd);
		}
		/* last byte? */
		if (dcnt == secsz - 1) {
			if (!mflag) {			/* single sector */
				state = FDC_IDLE;	/* done */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0;
			} else {			/* multiple sectors */
				if ((fdc_track == 0) && (side == 0))
					lastsec = disks[disk].sec0;
				else
					lastsec = disks[disk].sectors;
				if (lastsec == fdc_sec) {
					state = FDC_IDLE;  /* done */
					fdc_flags |= 1;    /* set EOJ */
					fdc_flags &= ~128; /* reset DRQ */
					fdc_stat = 0x10;   /* sector not found*/
				} else {
					dcnt = 0;	/* read next sector */
					fdc_sec++;
					return buf[secsz - 1];
				}
			}
		}
		/* return next byte from buffer and increment counter */
		return buf[dcnt++];

	case FDC_READADR:	/* read disk address */
		/* first byte? */
		if (dcnt == 0) {		/* build address field */
			buf[0] = fdc_track;
			buf[1] = side;
			buf[2] = fdc_sec;
			buf[3] = (ddens == SINGLE) ? 0 : 2;
			buf[4] = 0;
			buf[5] = 0;
		}
		/* last byte? */
		if (dcnt == 5) {
			state = FDC_IDLE;	/* done */
			fdc_flags |= 1;		/* set EOJ */
			fdc_flags &= ~128;	/* reset DRQ */
			fdc_stat = 0;
		}
		/* return next byte from buffer and increment counter */
		return buf[dcnt++];

	default:
		return (BYTE) 0;
	}
}

/*
 *	write data to FDC
 */
void cromemco_fdc_data_out(BYTE data)
{
	off_t pos;		/* seek position */
	int lastsec;		/* last sector of a track */
	static int wrtstat;	/* state while writing (formatting) tracks */
	static int bcnt;	/* byte counter for sector data */
	static int secs;	/* # of sectors written so far */

	switch (state) {
	case FDC_WRITE:			/* write data to disk sector */
		/* first byte? */
		if (dcnt == 0) {
			motortimer = 800;
			if (disks[disk].fn == NULL) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0x80;	/* not ready */
				return;
			}
			/* try to open disk image */
			dsk_path();
			strcat(fn, "/");
			strcat(fn, disks[disk].fn);
			if ((fd = open(fn, O_RDWR)) == -1) {
				if ((fd = open(fn, O_RDONLY)) != -1) {
					close(fd);
					fdc_stat = 0x40; /* read only */
				} else {
					fdc_stat = 0x80; /* not ready */
				}
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				return;
			}
			/* get drive and disk geometry */
			config_disk(fd);
			if (disks[disk].disk_t == UNKNOWN) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0x10;	/* sector not found */
				close(fd);
				return;
			}
			/* check track/sector */
			if ((fdc_track == 0) && (side == 0))
				lastsec = disks[disk].sec0;
			else
				lastsec = disks[disk].sectors;
			if ((fdc_track >= disks[disk].tracks) ||
			    (fdc_sec == 0) ||
			    (fdc_sec > lastsec)) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0x10;	/* sector not found */
				close(fd);
				return;
			}
			/* seek to sector */
			pos = get_pos();
			if (lseek(fd, pos, SEEK_SET) == -1L) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0x10;	/* sector not found */
				close(fd);
				return;
			}
		}
		/* write data bytes into the sector buffer */
		buf[dcnt++] = data;
		/* last byte? */
		if (dcnt == secsz) {
			state = FDC_IDLE;		/* done */
			fdc_flags |= 1;			/* set EOJ */
			fdc_flags &= ~128;		/* reset DRQ */
			if (write(fd, buf, secsz) == secsz)
				fdc_stat = 0;
			else
				fdc_stat = 0x20;	/* write fault */
			close(fd);
		}
		break;

	case FDC_WRTTRK:		/* write (format) track */
		if (dcnt == 0) {
			motortimer = 800;
			/* unlink disk image */
			dsk_path();
			strcat(fn, "/");
			strcat(fn, disks[disk].fn);
			if ((fdc_track == 0) && (side == 0))
				unlink(fn);
			/* try to create new disk image */
			if ((fd = open(fn, O_RDWR | O_CREAT, 0644)) == -1) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0x80;	/* not ready */
				return;
			}
			/* set initial drive and disk geometry */
			if ((fdc_track == 0) && (side == 0)) {
				if (dtype == LARGE) {
					disks[disk].disk_t = LARGE;
					disks[disk].disk_d = SINGLE;
					disks[disk].disk_s = ONE;
					disks[disk].tracks = TRK8;
					disks[disk].sectors = SPT8SD;
					disks[disk].sec0 = SPT8SD;
				} else {
					disks[disk].disk_t = SMALL;
					disks[disk].disk_d = SINGLE;
					disks[disk].disk_s = ONE;
					disks[disk].tracks = TRK5;
					disks[disk].sectors = SPT5SD;
					disks[disk].sec0 = SPT5SD;
				}
			}
			/* don't format tracks out of bounds */
			if (fdc_track >= disks[disk].tracks) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0;
				close(fd);
				return;
			}
			/* now learn more */
			if (side == 1)
				disks[disk].disk_s = TWO;
			if (ddens == DOUBLE) {
				disks[disk].disk_d = DOUBLE;
				if (dtype == LARGE) {
					disks[disk].sectors = SPT8DD;
				} else {
					disks[disk].sectors = SPT5DD;
				}
			}
			/* seek to track */
			fdc_sec = 1;
			pos = get_pos();
			if (lseek(fd, pos, SEEK_SET) == -1L) {
				state = FDC_IDLE;	/* abort command */
				fdc_flags |= 1;		/* set EOJ */
				fdc_flags &= ~128;	/* reset DRQ */
				fdc_stat = 0;
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
			if ((data != 0xf7) && (bcnt < secsz)) {
				buf[bcnt++] = data;
				return;
			} else {
				secs++;
				if (write(fd, buf, bcnt) == bcnt)
					fdc_stat = 0;
				else
					fdc_stat = 0x20; /* write fault */
				wrtstat = 1;
			}
		}
		/* all sectors of track written? */
		if ((fdc_track == 0) && (side == 0))
			lastsec = disks[disk].sec0;
		else
			lastsec = disks[disk].sectors;
		if (secs == lastsec) {
			state = FDC_IDLE;
			fdc_flags |= 1;		/* set EOJ */
			fdc_flags &= ~128;	/* reset DRQ */
			close(fd);
		}
		break;

	default:			/* track # for seek */
		if (fdc_track != data)
			step_dir = (data < fdc_track) ? -1 : 1;
		fdc_track = data;
		break;
	}
}

/*
 *	read FDC sector register
 */
BYTE cromemco_fdc_sector_in(void)
{
	return fdc_sec;
}

/*
 *	write FDC sector register
 */
void cromemco_fdc_sector_out(BYTE data)
{
	fdc_sec = data;
}

/*
 *	read FDC track register
 */
BYTE cromemco_fdc_track_in(void)
{
	return fdc_track;
}

/*
 *	write FDC track register
 *
 * Note: this does not set the FDC track register for a seek, the wanted
 * track # for a seek is written to the data port, see above.
 */
void cromemco_fdc_track_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	4FDC			16FDC
 * D7	DRQ			DRQ
 * D6	SEEK IN PROGRESS	SEEK IN PROGRESS
 * D5	unassigned		unassigned
 * D4	unassigned		unassigned
 * D3	unassigned		!FP Sense Switch 5
 * D2	unassigned		!FP Sense Switch 6
 * D1	unassigned		!FP Sense Switch 7
 * D0	unassigned		!FP Sense Switch 8
 *
 * SEEK IN PROGRESS is never set, we have no moving parts here.
 */
BYTE cromemco_fdc_aux_in(void)
{
	fdc_aux = 0;

	/* get DRQ from flag register */
	if (fdc_flags & 128) {
		fdc_aux |= 128;
	} else {
		fdc_aux &= ~128;
	}

#ifdef FRONTPANEL
	if (F_flag) {
		/* get front panel switch bits */
		if ((address_switch >> 8) & 16)
			fdc_aux &= ~8;
		else
			fdc_aux |= 8;

		if ((address_switch >> 8) & 32)
			fdc_aux &= ~4;
		else
			fdc_aux |= 4;

		if ((address_switch >> 8) & 64)
			fdc_aux &= ~2;
		else
			fdc_aux |= 2;

		if ((address_switch >> 8) & 128)
			fdc_aux &= ~1;
		else
			fdc_aux |= 1;
	} else {
#endif
		/* get front panel switch bits */
		if (fp_port & 16)
			fdc_aux &= ~8;
		else
			fdc_aux |= 8;

		if (fp_port & 32)
			fdc_aux &= ~4;
		else
			fdc_aux |= 4;

		if (fp_port & 64)
			fdc_aux &= ~2;
		else
			fdc_aux |= 2;

		if (fp_port & 128)
			fdc_aux &= ~1;
		else
			fdc_aux |= 1;
#ifdef FRONTPANEL
	}
#endif

	return fdc_aux;
}

/*
 *	4FDC		16FDC
 * D7	unassigned	unassigned
 * D6	!EJECT LEFT	!EJECT
 * D5	!EJECT RIGHT	!DRIVE SELECT OVERRIDE
 * D4	!FAST SEEK	!FAST SEEK
 * D3	!RESTORE	!RESTORE
 * D2	!CONTROL OUT	!CONTROL OUT
 * D1	unassigned	!SIDE SELECT
 * D0	unassigned	unassigned
 */
void cromemco_fdc_aux_out(BYTE data)
{
	/* fast seek for the PERSCI disk drives */
	if (!(data & 8))
		fdc_track = 0;

	/* get disk side */
	if (!(data & 2))
		side = 1;
	else
		side = 0;
}

/*
 *	read disk status
 *
 * Depends on the last command, WD status table:
 *
 * Command	D7	D6	D5	D4	D3	D2	D1	D0
 * SEEK		not	write	head	not	CRC	track	index	busy
 *		ready	protect	down	found	error	0
 *
 * STEP		not	write	head	not	CRC	track	index	busy
 * RESTORE	ready	protect	down	found	error	0
 *
 * READ		not	0	record	not	CRC	lost	DRQ	busy
 * RECORD(S)	ready		type	found	error	data
 *
 * WRITE	not	write	0	not	CRC	lost	DRQ	busy
 * RECORD(S)	read	protect		found	error	data
 *
 * READ		not	0	0	not	CRC	lost	DRQ	busy
 * ADDRESS	ready			found	error	data
 *
 * READ		not	0	0	0	0	lost	DRQ	busy
 * TRACK	ready					data
 *
 * WRITE	not	write	0	0	0	lost	DRQ	busy
 * TRACK	ready	protect				data
 */
BYTE cromemco_fdc_status_in(void)
{
	/* set/reset the index pulse, write protect and track 0 bits
	   depending on last command */
	if (((fdc_cmd & 0xf0) == 0) ||		/* restore (seek track 0) */
	    ((fdc_cmd & 0xf0) == 0x10) ||	/* seek */
	    ((fdc_cmd & 0xe0) == 0x20) ||	/* step */
	    ((fdc_cmd & 0xe0) == 0x40) ||	/* step in */
	    ((fdc_cmd & 0xe0) == 0x60)) {	/* step out */

		if (index_pulse)
			fdc_stat |= 2;
		else
			fdc_stat &= ~2;

		if (disks[disk].disk_m == READWRITE)
			fdc_stat &= ~64;
		else
			fdc_stat |= 64;

		if (fdc_track == 0)
			fdc_stat |= 4;
		else
			fdc_stat &= ~4;
	}

	return fdc_stat;
}

/*
 *	write disk command
 */
void cromemco_fdc_cmd_out(BYTE data)
{
	/* if controller busy ignore all commands but interrupt */
	if ((fdc_stat & 1) && ((data & 0xf0) != 0xd0))
		return;

	/* new command, save it and reset EOJ */
	fdc_cmd = data;
	fdc_flags &= ~1;

	if ((data & 0xf0) == 0) {		/* restore (seek track 0) */
		headloaded = (data & 8) ? 1 : 0;
		if (fdc_track != 0)
			step_dir = -1;
		fdc_track = 0;
		fdc_flags |= 1;			/* set EOJ */
		fdc_stat = 4;			/* positioned to track 0 */
		if (headloaded)
			fdc_stat |= 32;

	} else if ((data & 0xf0) == 0x10) {	/* seek */
		headloaded = (data & 8) ? 1 : 0;
		fdc_flags |= 1;			/* set EOJ */
		if (fdc_track <= disks[disk].tracks)
			fdc_stat = (fdc_track == 0) ? 4 : 0;
		else
			fdc_stat = 16;		/* seek error */
		if (headloaded)
			fdc_stat |= 32;

	} else if ((data & 0xe0) == 0x20) {	/* step */
		headloaded = (data & 8) ? 1 : 0;
		fdc_track += step_dir;
		fdc_flags |= 1;			/* set EOJ */
		if (fdc_track <= disks[disk].tracks)
			fdc_stat = (fdc_track == 0) ? 4 : 0;
		else
			fdc_stat = 16;		/* seek error */
		if (headloaded)
			fdc_stat |= 32;

	} else if ((data & 0xe0) == 0x40) {	/* step in */
		headloaded = (data & 8) ? 1 : 0;
		fdc_track++;
		fdc_flags |= 1;			/* set EOJ */
		step_dir = 1;
		if (fdc_track <= disks[disk].tracks)
			fdc_stat = (fdc_track == 0) ? 4 : 0;
		else
			fdc_stat = 16;		/* seek error */
		if (headloaded)
			fdc_stat |= 32;

	} else if ((data & 0xe0) == 0x60) {	/* step out */
		headloaded = (data & 8) ? 1 : 0;
		fdc_track--;
		fdc_flags |= 1;			/* set EOJ */
		step_dir = -1;
		if (fdc_track <= disks[disk].tracks)
			fdc_stat = (fdc_track == 0) ? 4 : 0;
		else
			fdc_stat = 16;		/* seek error */
		if (headloaded)
			fdc_stat |= 32;

	} else if ((data & 0xf0) == 0xd0) {	/* force interrupt */
		state = FDC_IDLE;		/* abort any command */
		fdc_stat = 0;
		fdc_flags &= ~128;		/* clear DRQ */
		if (data & 0x0f)
			fdc_flags |= 1;		/* set EOJ */

	} else if ((data & 0xe0) == 0x80) {	/* read sector(s) */
		mflag = (data & 16) ? 1 : 0;
		state = FDC_READ;
		dcnt = 0;
		fdc_stat = 3;			/* set DRQ & busy */
		fdc_flags |= 128;		/* set DRQ */

	} else if ((data & 0xf0) == 0xa0) {	/* write single sector */
		state = FDC_WRITE;
		dcnt = 0;
		fdc_stat = 3;			/* set DRQ & busy */
		fdc_flags |= 128;		/* set DRQ */

	} else if (data == 0xc4) {		/* read address */
		state = FDC_READADR;
		dcnt = 0;
		fdc_stat = 3;			/* set DRQ & busy */
		/*fdc_flags |= 128;*/		/* set DRQ */
		fdc_flags |= 129; /* RDOS 2&3 seem to need EOJ already, why */

	} else if ((data & 0xf0) == 0xe0) {	/* read track */
		LOGW(TAG, "read track not implemented");
		fdc_stat = 16;			/* not found */
		fdc_flags |= 1;			/* set EOJ */

	} else if ((data & 0xf0) == 0xf0) {	/* write track */
		state = FDC_WRTTRK;
		dcnt = 0;
		fdc_stat = 3;			/* set DRQ & busy */
		fdc_flags |= 128;		/* set DRQ */

	} else {
		LOGW(TAG, "unknown command %02x", data);
		fdc_stat = 16 | 8;		/* not found & CRC error */
		fdc_flags |= 1;			/* set EOJ */
	}
}

/*
 * Reset FDC
 */

void cromemco_fdc_reset(void)
{
	state = dcnt = mflag = index_pulse = disk = side = 0;
	motoron = motortimer = headloaded = autowait = 0;
	fdc_stat = fdc_aux = 0;
	fdc_flags = AUTOBOOT;
	secsz = SEC_SZSD;

#ifdef HAS_BANKED_ROM
	if (R_flag) {
		fdc_rom_active = 1;
		reset_fdc_rom_map();
	} else {
		fdc_rom_active = 0;
	}
#endif

	readDiskmap(dsk_path());
}
