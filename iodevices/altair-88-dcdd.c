/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2018 by Udo Munk
 *
 * Emulation of the MITS Altair S100 floppy disk controller
 *
 * History:
 */

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sim.h"
#include "simglb.h"
//#define LOG_LOCAL_LEVEL LOG_DEBUG
#include "log.h"

/* internal state of the fdc */
#define FDC_DISABLED	0	/* FDC and disk are disabled */
#define FDC_ENABLED	1	/* FDC and disk are enabled */

/* fdc status bits */
#define ENWD	1		/* enter new write data */
#define MOVEHD	2		/* indicates when head movement allowed */
#define STATHD	4		/* indicates when head properly loaded */
#define INTE	32		/* CPU INTE */
#define TRACK0	64		/* track 0 detected */
#define NRDA	128		/* new read data available */

/* disk format */
#define SEC_SZ		137
#define SPT		32
#define TRK		77

static const char *TAG = "88DCDD";

static int track;		/* current track */
static int sec;			/* current sector position */
static int rwsec;		/* sector read/written */
static int disk;		/* current disk # */
static BYTE status = 0xff;	/* controller status */
static int headloaded;		/* head loaded flag */
static int state;		/* fdc state */
static char fn[MAX_LFN];	/* path/filename for disk image */
static int fd;			/* fd for disk file i/o */
static int dcnt;		/* data counter read/write */
static BYTE buf[SEC_SZ];	/* buffer for one sector */

static int cnt_sec;		/* counter for sector position */
static int cnt_head;		/* counter for loading head */
static int cnt_step;		/* counter for stepping track */

static pthread_t thread;	/* thread for timing */

/* these are our disk drives */
static char *disks[16] = {
	"drivea.dsk",
	"driveb.dsk",
	"drivec.dsk",
	"drived.dsk",
	"drivee.dsk",
	"drivef.dsk",
	"driveg.dsk",
	"driveh.dsk",
	"drivei.dsk",
	"drivej.dsk",
	"drivek.dsk",
	"drivel.dsk",
	"drivem.dsk",
	"driven.dsk",
	"driveo.dsk",
	"drivep.dsk"
};

/*
 * find and set path for disk images
 */
static void dsk_path(void)
{
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
 * check disk image
 */
static int dsk_check(void)
{
	struct stat s;

	/* try to open disk image */
	dsk_path();
	strcat(fn, "/");
	strcat(fn, disks[disk]);
	if ((fd = open(fn, O_RDONLY)) == -1)
		return(0);

	/* check for correct image size */
	fstat(fd, &s);
	close(fd);
	if (s.st_size != 337568)
		return(0);
	else
		return(1);
}

/*
 * disable disk system
 */
static void dsk_disable(void)
{
	state = FDC_DISABLED;
	status = 0xff;
	headloaded = 0;
	if (thread != 0) {
		pthread_cancel(thread);
		pthread_join(thread, NULL);
		thread = 0;
	}
	LOGD(TAG, "disabled");
}

/*
 * thread for timing
 */
static void *timing(void *arg)
{
	arg = arg;	/* to avoid compiler warning */

	/* 1 msec per loop iteration */
	while (1) {
		/* advance sector position */
		if (++cnt_sec > 5) {	/* 5ms for each sector */
			cnt_sec = 0;
			if (++sec >= SPT) {
				sec = 0;
			}
		}

		/* count down head load timer */
		if (cnt_head > 0) {
			if (--cnt_head == 0) {
				status &= ~STATHD;
				LOGD(TAG, "head loaded");
			}
		}

		/* count down stepping timer */
		if (cnt_step > 0) {
			if (--cnt_step == 0) {
				status &= ~MOVEHD;
			}
		}

		/* sleep for 1 millisecond */
		SLEEP_MS(1);
	}

	pthread_exit(NULL);
}

/*
 * enables/disables controller and selects disk drives
 */
void altair_dsk_select_out(BYTE data)
{
	/* disable? */
	if (data & 0x80) {
		dsk_disable();
	/* no, enable */
	} else {
		/* get disk no. */
		disk = data & 0x0f;
		/* check disk in drive */
		if (dsk_check() == 0) {
			/* no (valid) disk in drive, disable */
			dsk_disable();
			return;
		}
		/* enable */
		state = FDC_ENABLED;
		status = 0b10100101;
		if (thread == 0) {
			if (pthread_create(&thread, NULL, timing,
			    (void *) NULL)) {
				LOGE(TAG, "can't create timing thread");
				exit(1);
			}
		}
		LOGD(TAG, "enabled, disk = %d", disk);
	}
}

/*
 * Status when drive and controller enabled
 * True condition = 0, False = 1
 *
 * D0 enter new write data
 * D1 indicates when head movement allowed
 * D2 indicates when head is properly loaded
 * D3 not used, = 0 (software uses this to figure if enabled!)
 * D4 not used, = 0
 * D5 CPU INTE
 * D6 track 0 detected
 * D7 new read data available
 */
BYTE altair_dsk_status_in(void)
{
	if (state == FDC_ENABLED) {
		/* set CPU INTE */
		if (IFF & 1)
			status &= ~INTE;
		else
			status |= INTE;

		/* set track 0 */
		if (track == 0)
			status &= ~TRACK0;
		else
			status |= TRACK0;
	}

	return(status);
}

/*
 * Controls disk operations when disk drive and controller enabled:
 *
 * D0 Step in		- steps disk head in to higher numbered track
 * D1 Step out		- steps disk head out to lower numbered track
 * D2 Head load		- loads head, enables sector position status
 * D3 Head unload	- removes head
 * D4 Interrupt enable	- n.i.
 * D5 Interrupt disable - n.i.
 * D6 Head current switch - don't care
 * F7 Write enable	- start writing sector
 */
void altair_dsk_control_out(BYTE data)
{
	if (state == FDC_ENABLED) {
		/* step in */
		if (data & 1) {
			LOGD(TAG, "step in from track %d", track);
			if (track < (TRK - 1)) {
				track++;
				status |= MOVEHD;
				cnt_step = 10;
				/* head needs to settle again */
				if (headloaded) {
					status |= STATHD;
					cnt_head = 40;
				}
			}
		}

		/* step out */
		if (data & 2) {
			LOGD(TAG, "step out from track %d", track);
			if (track > 0) {
				track--;
				status |= MOVEHD;
				cnt_step = 10;
				/* head needs to settle again */
				if (headloaded) {
					status |= STATHD;
					cnt_head = 40;
				}
			}
		}

		/* load head */
		if (data & 4) {
			headloaded = 1;
			cnt_head = 40;
			status |= MOVEHD;
			cnt_step = 40;
			LOGD(TAG, "load head");
		}

		/* unload head */
		if (data & 8) {
			headloaded = 0;
			cnt_head = 0;
			status |= STATHD;
			LOGD(TAG, "unload head");
		}
	}
}

/*
 * FDC sector position
 */
BYTE altair_dsk_sec_in(void)
{
	BYTE sectrue;

	if ((state != FDC_ENABLED) || (status & STATHD)) {
		status |= NRDA;
		return(0xff);
	} else {
		if (sec != rwsec) {
			rwsec = sec;
			sectrue = 0;	/* start of new sector */
			status &= ~NRDA; /* new read data available */
			dcnt = 0;
		} else {
			sectrue = 1;
		}
	}

	return(sectrue + (rwsec << 1));
}

/*
 * FDC data output port
 */
void altair_dsk_data_out(BYTE data)
{
}

/*
 * FDC data input port
 */
BYTE altair_dsk_data_in(void)
{
	BYTE data;
	long pos;

	/* first byte? */
	if (dcnt == 0) {
		/* check disk */
		if (dsk_check() == 0) {
			dsk_disable();
			memset(&buf[0], 0xff, SEC_SZ);
		} else {
			/* read sector */
			fd = open(fn, O_RDONLY);
			pos = (track * SPT + rwsec) * SEC_SZ;
			lseek(fd, pos, SEEK_SET);
			read(fd, &buf[0], SEC_SZ);
			close(fd);
			LOGD(TAG, "read sector %d track %d", rwsec, track);
		}
	}

	/* no more data? */
	if (dcnt == SEC_SZ)
		return(0xff);

	/* return byte from buffer and increment counter */
	data = buf[dcnt++];
	if (dcnt == SEC_SZ)
		status |= NRDA;	/* no more data to read */
	return(data);
}
