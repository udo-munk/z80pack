/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2019 by Udo Munk
 *
 * Emulation of an IMSAI FIF S100 board
 *
 * History:
 * 18-JAN-2014 first working version finished
 * 02-MAR-2014 improvements
 * 23-MAR-2014 got all 4 disk drives working
 *    AUG-2014 some improvements after seeing the original IMSAI CP/M 1.3 BIOS
 * 17-SEP-2014 FDC error 9 for DMA overrun, as reported by Alan Cox for cpmsim
 * 27-JAN-2015 unlink and create new disk file if format track command
 * 08-MAR-2016 support user path for disk images
 * 13-MAY-2016 find disk images at -d <path>, ./disks and DISKDIR
 * 22-JUL-2016 added support for read only disks
 * 07-DEC-2016 added bus request for the DMA
 * 19-DEC-2016 use the new memory interface for DMA access
 * 22-JUN-2017 added reset function
 * 19-MAY-2018 improved reset
 * 13-JUL-2018 use logging & integrate disk manager
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "sim.h"
#include "simglb.h"
#include "config.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"
/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"

/* offsets in disk descriptor */
#define DD_UNIT		0	/* unit/command */
#define DD_RESULT	1	/* result code */
#define DD_NN		2	/* track number high, not used */
#define DD_TRACK	3	/* track number low */
#define DD_SECTOR	4	/* sector number */
#define DD_DMAL		5	/* DMA address low */
#define DD_DMAH		6	/* DMA address high */

/* FD command in disk descriptor unit/command field */
#define WRITE_SEC	1
#define READ_SEC	2
#define FMT_TRACK	3
#define VERIFY_DATA	4

/* sector size */
#define SEC_SZ		128

/* 8" standard disks */
#define SPT8		26
#define TRK8		77

static const char *TAG = "FIF";

#ifdef HAS_DISKMANAGER
extern char *disks[];
#else
/* these are our disk drives */
static char *disks[4] = {
	"drivea.dsk",
	"driveb.dsk",
	"drivec.dsk",
	"drived.dsk"
};
#endif

static char fn[MAX_LFN];	/* path/filename for disk image */
static int fdstate = 0;		/* state of the fd */

/*
 * find and set path for disk images
 */
char *dsk_path(void) {
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
		strncpy(diskd, fn, MAX_LFN);
	}
	return diskd;
}

BYTE imsai_fif_in(void)
{
	return(0);
}

void imsai_fif_out(BYTE data)
{
	static int fdaddr[16];		/* address of disk descriptors */
	static int descno;		/* descriptor # */

	void disk_io(int);

	/*
	 * The controller understands these commands:
	 * 0x10: set address of a disk descriptor from the following two out's
	 * 0x00: do the work as setup in the disk descriptor
	 * 0x20: reset a drive to home position, the lower digit contains
	 *       the drive to reset, 0x2f for all drives
	 *
	 * The dd address only needs to be set once, the OS then can adjust
	 * the wanted I/O in the descriptor and send the 0x00 command
	 * multiple times for this descriptor.
	 *
	 * The commands 0x10 and 0x00 are OR'ed with a descriptor number
	 * 0x0 - 0xf, so there can be 16 different disk descriptors that
	 * need to be remembered.
	 */
	switch (fdstate) {
	case 0:	/* start of command phase */
		switch (data & 0xf0) {
		case 0x00:	/* do what disk descriptor says */
			descno = data & 0xf;
			disk_io(fdaddr[descno]);
			break;

		case 0x10:	/* next 2 is address of disk descriptor */
			descno = data & 0xf;
			fdstate++;
			break;
	
		case 0x20:	/* reset drive(s) */
			break;	/* no mechanical drives, so nothing to do */

		default:
			LOGW(TAG, "unknown cmd %02x", data);
			return;
		}
		break;

	case 1: /* LSB of disk descriptor address */
		fdaddr[descno] = data;
		fdstate++;
		break;

	case 2: /* MSB of disk descriptor address */
		fdaddr[descno] += data << 8;
		fdstate = 0;
		break;

	default:
		LOGE(TAG, "internal state error");
		cpu_error = IOERROR;
		cpu_state = STOPPED;
		break;
	}
}

/*
 *	Here we do the disk I/O.
 *
 *	The status byte in the disk descriptor is set as follows:
 *
 *	1 - OK
 *	2 - illegal drive
 *	3 - no disk in drive
 *	4 - illegal track
 *	5 - illegal sector
 *	6 - seek error
 *	7 - read error
 *	8 - write error
 *	15 - invalid command
 *
 *	All error codes will abort disk I/O, but these are not the ones
 *	the real controller will set.
 *	For example the real controller will set 0A1H for drive not
 *	ready and the IMSAI BIOS waits forever, until a disk is inserted
 *	and the drive door closed.
 */
void disk_io(int addr)
{
	register int i;
	static int fd;			/* fd for disk i/o */
	static long pos;		/* seek position */
	static int unit;		/* disk unit number */
	static int cmd;			/* disk command */
	static int track;		/* disk track */
	static int sector;		/* disk sector */
	static int dma_addr;		/* DMA address */
	static int spt;			/* sectors per track */
	static int maxtrk;		/* max tracks of disk */
	static int disk;		/* internal disk no */
	static char blksec[SEC_SZ];

	LOGD(TAG, "disk descriptor at %04x", addr);
	LOGD(TAG, "unit: %02x", *(mem_base() + addr + DD_UNIT));
	LOGD(TAG, "result: %02x", *(mem_base() + addr + DD_RESULT));
	LOGD(TAG, "nn: %02x", *(mem_base() + addr + DD_NN));
	LOGD(TAG, "track: %02x", *(mem_base() + addr + DD_TRACK));
	LOGD(TAG, "sector: %02x", *(mem_base() + addr + DD_SECTOR));
	LOGD(TAG, "DMA low: %02x", *(mem_base() + addr + DD_DMAL));
	LOGD(TAG, "DMA high: %02x", *(mem_base() + addr + DD_DMAH));
	LOGD(TAG, "");

	unit = dma_read(addr + DD_UNIT) & 0xf;
	cmd = dma_read(addr + DD_UNIT) >> 4;
	track = dma_read(addr + DD_TRACK);
	sector = dma_read(addr + DD_SECTOR);
	dma_addr = (dma_read(addr + DD_DMAH) << 8) + dma_read(addr + DD_DMAL);

	/* convert IMSAI unit bits to internal disk no */
	switch (unit) {
	case 1:	/* IMDOS drive A: */
		spt = SPT8;
		maxtrk = TRK8;
		disk = 0;
		break;

	case 2:	/* IMDOS drive B: */
		spt = SPT8;
		maxtrk = TRK8;
		disk = 1;
		break;

	case 4: /* IMDOS drive C: */
		spt = SPT8;
		maxtrk = TRK8;
		disk = 2;
		break;

	case 8: /* IMDOS drive D: */
		spt = SPT8;
		maxtrk = TRK8;
		disk = 3;
		break;

	default: /* set error code for all other drives */
		 /* IMDOS sends unit 3 intermediate for drive C: & D: */
		 /* and the IMDOS format program sends unit 0 */
		dma_write(addr + DD_RESULT, 2);
		return;
	}

	/* handle case when disk is ejected */
	if(disks[disk] == NULL) {
		dma_write(addr + DD_RESULT, 3);
		return;
	}

	/* try to open disk image for the wanted operation */
	dsk_path();
	strcat(fn, "/");
	strcat(fn, disks[disk]);
	if (cmd == FMT_TRACK) {
		if (track == 0)
			unlink(fn);
		fd = open(fn, O_RDWR|O_CREAT, 0644);
	} else if (cmd == READ_SEC) {
		fd = open(fn, O_RDONLY);
	} else {
		fd = open(fn, O_RDWR);
	}
	if (fd == -1) {
		dma_write(addr + DD_RESULT, 3);
		return;
	}

	/* we have a disk, try wanted disk operation */
	switch(cmd) {
	case WRITE_SEC:
		if (track >= maxtrk) {
			dma_write(addr + DD_RESULT, 4);
			goto done;
		}
		if (sector > spt) {
			dma_write(addr + DD_RESULT, 5);
			goto done;
		}
		pos = (track * spt + sector - 1) * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == -1L) {
			dma_write(addr + DD_RESULT, 6);
			goto done;
		}
		bus_request = 1;
		for (i = 0; i < SEC_SZ; i++)
			blksec[i] = dma_read(dma_addr + i);
		if (write(fd, blksec, SEC_SZ) != SEC_SZ) {
			dma_write(addr + DD_RESULT, 8);
			goto done;
		}
		bus_request = 0;
		dma_write(addr + DD_RESULT, 1);
		break;

	case READ_SEC:
		if (track >= maxtrk) {
			dma_write(addr + DD_RESULT, 4);
			goto done;
		}
		if (sector > spt) {
			dma_write(addr + DD_RESULT, 5);
			goto done;
		}
		pos = (track * spt + sector - 1) * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == -1L) {
			dma_write(addr + DD_RESULT, 6);
			goto done;
		}
		bus_request = 1;
		if (read(fd, blksec, SEC_SZ) != SEC_SZ) {
			dma_write(addr + DD_RESULT, 7);
			goto done;
		}
		for (i = 0; i < SEC_SZ; i++)
			dma_write(dma_addr + i, blksec[i]);
		bus_request = 0;
		dma_write(addr + DD_RESULT, 1);
		break;

	case FMT_TRACK:
		memset(&blksec, 0xe5, SEC_SZ);
		if (track >= maxtrk) {
			dma_write(addr + DD_RESULT, 4);
			goto done;
		}
		pos = track * spt * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == -1L) {
			dma_write(addr + DD_RESULT, 6);
			goto done;
		}
		bus_request = 1;
		for (i = 0; i < spt; i++) {
			if (write(fd, &blksec, SEC_SZ) != SEC_SZ) {
				dma_write(addr + DD_RESULT, 8);
				goto done;
			}
		}
		bus_request = 0;
		dma_write(addr + DD_RESULT, 1);
		break;

	case VERIFY_DATA: /* emulated disks are reliable, just report ok */
		dma_write(addr + DD_RESULT, 1);
		break;

	default:	/* unknown command */
		dma_write(addr + DD_RESULT, 15);
		break;
	}

done:
	bus_request = 0;
	close(fd);
}

/*
 * Reset FDC
 */
void imsai_fif_reset(void)
{
	fdstate = 0;

#ifdef HAS_DISKMANAGER
	extern void readDiskmap(char *);
	readDiskmap(dsk_path());
#endif
}
