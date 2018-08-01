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
#include "sim.h"
#include "simglb.h"
#include "log.h"

/* internal state of the fdc */
#define FDC_DISABLED	0	/* FDC and disk are disabled */
#define FDC_ENABLED	1	/* FDC and disk are enabled */

/* disk format */
#define SEC_SZ		137
#define SPT		32
#define TRK		77

static const char *TAG = "88DCDD";

static int track;		/* current track */
static int sec;			/* current sector position */
static int disk;		/* current disk # */
static int headloaded;		/* disk head loaded */
static int state;		/* fdc state */
static char fn[MAX_LFN];	/* path/filename for disk image */
static int fd;			/* fd for disk file i/o */

static int cnt_sec;		/* counter for sector position */

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
static void dsk_path(void) {
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
 * thread for timing
 */
static void *timing(void *arg)
{
	arg = arg;	/* to avoid compiler warning */

	while (1) {	/* 1 msec per loop iteration */
		/* advance sector position */
		if (++cnt_sec >= 5) {	/* 5ms for each sector */
			cnt_sec = 0;
			if (++sec >= SPT)
				sec = 0;
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
	if (data & 0x80) {
		state = FDC_DISABLED;
		if (thread != 0) {
			state = FDC_DISABLED;
			pthread_cancel(thread);
			pthread_join(thread, NULL);
			thread = 0;
		}
	} else {
		state = FDC_ENABLED;
		disk = data & 0x0f;
		if (thread == 0) {
			if (pthread_create(&thread, NULL, timing,
			    (void *) NULL)) {
				LOGE(TAG, "can't create timing thread");
				exit(1);
			}
		}
	}
}

BYTE altair_dsk_status_in(void)
{
	return(0xff);
}

/*
 * Controls disk operations when disk drive and controller enabled:
 *
 * D0 Step in		- steps disk head to higher numbered track
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
	if (data & 4)
		headloaded = 1;
	if (data & 8)
		headloaded = 0;
}

/*
 * sector position
 */
BYTE altair_dsk_sec_in(void)
{
	BYTE data;

	if ((state != FDC_ENABLED) || (headloaded == 0))
		return(0xff);
	else {
		if (cnt_sec == 0)	/* at begin of setor */
			data = 0;	/* sector true */
		else
			data = 1;	/* sector false */
		data += sec << 1;
	}

	return(data);
}

void altair_dsk_data_out(BYTE data)
{
}

BYTE altair_dsk_data_in(void)
{
	return(0xff);
}
