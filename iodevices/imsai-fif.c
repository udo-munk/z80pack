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
 * 10-SEP-2019 added support for a z80pack 4 MB harddisk
 * 04-NOV-2019 eliminate usage of mem_base() & remove fake bus_request
 * 17-NOV-2019 return result codes as documented in manual
 * 18-NOV-2019 initialize command string address array
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

/* support a z80pack 4 MB drive */
#define LARGEDISK

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

#ifdef LARGEDISK
/* z80pack 4 MB drive */
#define SPTHD		128
#define TRKHD		255
#endif

static const char *TAG = "FIF";

#ifdef HAS_DISKMANAGER
extern char *disks[];
#else
/* these are our disk drives */
#ifdef LARGEDISK
static char *disks[9] = {
	"drivea.dsk",
	"driveb.dsk",
	"drivec.dsk",
	"drived.dsk",
	NULL,
	NULL,
	NULL,
	NULL,
	"drivei.dsk"
};
#else
static char *disks[4] = {
	"drivea.dsk",
	"driveb.dsk",
	"drivec.dsk",
	"drived.dsk"
};
#endif
#endif

static int fdaddr[16];		/* address of disk descriptors */
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
	static int descno;		/* descriptor # */

	void disk_io(int);

	/*
	 * controller commands: MSB command, LSB disk decsriptor or drive(s)
	 *
	 * 0x00: execute disk descriptor in LSB
	 * 0x10: set address of disk descriptor in LSB from following two out's
	 * 0x20: reset drives in LSB to home position
	 * 0x30: write protect drives in LSB
	 * 0x40: reset write protect for drives in LSB
	 * 0x50: reset and read boot sector from drive 0 into memory location 0
	 * 0x60 - 0xF0: perform no operation
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

		case 0x30:	/* write protect drive(s) */
			LOGW(TAG, "write protect not implemented");
			break;

		case 0x40:	/* reset write protect drive(s) */
			LOGW(TAG, "reset write protect not implemented");
			break;

		case 0x50:	/* reset and load boot sector into memory */
			LOGW(TAG, "read boot sector not implemented");
			break;

		default:	/* all other commands perform no operation */
			break;
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
 *	MSB = error class	LSB = error number
 *	1100 - class 1 error in command string
 *	1010 - class 2 operator recoverable error
 *	1001 - class 3 hardware failure
 *
 *	Class 1 - Bit 6 is set, status code has the form CXH
 *	C2 - no drive selected
 *	C3 - more than one drive selected
 *	C4 - illegal command number
 *	C5 - illegal track address
 *	C6 - illegal sector address
 *	C7 - illegal buffer address
 *	C8 - illegal byte-3 format
 *
 *	Class 2 - Bit 5 is set, status code has the form AXH
 *	A1 - selected drive not ready
 *	A2 - selected drive is hardware write protected
 *	A3 - selected drive is software write protected
 *	A4 - sector lenght specified by byte 3 of command string does
 *	     not correspond to actual sector lenght found on disk
 *
 *	Class 3 - Bit 4 is set, status code has the form 9XH
 *	91 - selected drive not operable
 *	92 - track address error while attempting to read/write
 *	93 - data synchronization error
 *	94 - CRC error in the ID field of desired sector
 *	95 - failure to recognize data AM after recognizing sector ID field
 *	96 - CRC error in the data field of desired sector
 *	97 - deleted data address mark in data field
 *	98 - format operation unsuccessful
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
	static struct stat s;
	static char blksec[SEC_SZ];

	LOGD(TAG, "disk descriptor at %04x", addr);
	LOGD(TAG, "unit: %02x", getmem(addr + DD_UNIT));
	LOGD(TAG, "result: %02x", getmem(addr + DD_RESULT));
	LOGD(TAG, "nn: %02x", getmem(addr + DD_NN));
	LOGD(TAG, "track: %02x", getmem(addr + DD_TRACK));
	LOGD(TAG, "sector: %02x", getmem(addr + DD_SECTOR));
	LOGD(TAG, "DMA low: %02x", getmem(addr + DD_DMAL));
	LOGD(TAG, "DMA high: %02x", getmem(addr + DD_DMAH));
	LOGD(TAG, "");

	unit = dma_read(addr + DD_UNIT) & 0xf;
	cmd = dma_read(addr + DD_UNIT) >> 4;
	track = dma_read(addr + DD_TRACK);
	sector = dma_read(addr + DD_SECTOR);
	dma_addr = (dma_read(addr + DD_DMAH) << 8) + dma_read(addr + DD_DMAL);

	/* convert IMSAI unit bits to internal disk no */
	switch (unit) {
	case 0: /* no drive selected */
		dma_write(addr + DD_RESULT, 0xc2);
		return;

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

#ifdef LARGEDISK
	case 15: /* z80pack 4 MB drive */
		spt = SPTHD;
		maxtrk = TRKHD;
		disk = 8;
		break;
#endif

	default: /* more than one drive selected */
		dma_write(addr + DD_RESULT, 0xc3);
		return;
	}

	/* handle case when disk is ejected */
	if(disks[disk] == NULL) {
		dma_write(addr + DD_RESULT, 0xa1);
		return;
	}

	/* try to open disk image */
	dsk_path();
	strcat(fn, "/");
	strcat(fn, disks[disk]);
	if (cmd == FMT_TRACK) {
		/* can only format floppy disks */
		if (disk <= 3) {
			if (track == 0)
				unlink(fn);
			fd = open(fn, O_RDWR|O_CREAT, 0644);
		} else {
			dma_write(addr + DD_RESULT, 0xa1);
			return;
		}
		if (fd == -1) {
			dma_write(addr + DD_RESULT, 0xa1);
			return;
		}
		goto do_format;
	} else if (cmd == READ_SEC) {
		fd = open(fn, O_RDONLY);
		if (fd == -1) {
			dma_write(addr + DD_RESULT, 0xa1);
			return;
		}
	} else {
		fd = open(fn, O_RDWR);
		if (fd == -1) {
			dma_write(addr + DD_RESULT, 0xa2);
			return;
		}
	}

	/* check for correct disk size if not formatting a new disk */
	fstat(fd, &s);
	if (((disk <= 3) && (s.st_size != 256256)) ||
	   ((disk == 8) && (s.st_size != 4177920))) {
		dma_write(addr + DD_RESULT, 0xa1);
		close(fd);
		return;
	}

do_format:

	/* try wanted disk operation */
	switch(cmd) {
	case WRITE_SEC:
		if (track >= maxtrk) {
			dma_write(addr + DD_RESULT, 0xc5);
			goto done;
		}
		if (sector > spt) {
			dma_write(addr + DD_RESULT, 0xc6);
			goto done;
		}
		pos = (track * spt + sector - 1) * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == -1L) {
			dma_write(addr + DD_RESULT, 0x92);
			goto done;
		}
		for (i = 0; i < SEC_SZ; i++)
			blksec[i] = dma_read(dma_addr + i);
		if (write(fd, blksec, SEC_SZ) != SEC_SZ) {
			dma_write(addr + DD_RESULT, 0x93);
			goto done;
		}
		dma_write(addr + DD_RESULT, 1);
		break;

	case READ_SEC:
		if (track >= maxtrk) {
			dma_write(addr + DD_RESULT, 0xc5);
			goto done;
		}
		if (sector > spt) {
			dma_write(addr + DD_RESULT, 0xc6);
			goto done;
		}
		pos = (track * spt + sector - 1) * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == -1L) {
			dma_write(addr + DD_RESULT, 0x92);
			goto done;
		}
		if (read(fd, blksec, SEC_SZ) != SEC_SZ) {
			dma_write(addr + DD_RESULT, 0x93);
			goto done;
		}
		for (i = 0; i < SEC_SZ; i++)
			dma_write(dma_addr + i, blksec[i]);
		dma_write(addr + DD_RESULT, 1);
		break;

	case FMT_TRACK:
		memset(&blksec, 0xe5, SEC_SZ);
		if (track >= maxtrk) {
			dma_write(addr + DD_RESULT, 0xc5);
			goto done;
		}
		pos = track * spt * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == -1L) {
			dma_write(addr + DD_RESULT, 0x92);
			goto done;
		}
		for (i = 0; i < spt; i++) {
			if (write(fd, &blksec, SEC_SZ) != SEC_SZ) {
				dma_write(addr + DD_RESULT, 0x93);
				goto done;
			}
		}
		dma_write(addr + DD_RESULT, 1);
		break;

	case VERIFY_DATA: /* emulated disks are reliable, just report ok */
		dma_write(addr + DD_RESULT, 1);
		break;

	default:	/* unknown command */
		dma_write(addr + DD_RESULT, 0xc4);
		break;
	}

done:
	close(fd);
}

/*
 * Reset FDC
 */
void imsai_fif_reset(void)
{
	fdstate = 0;

	fdaddr[0] = 0x0080;
	fdaddr[1] = 0x1000;
	fdaddr[2] = 0x2000;
	fdaddr[3] = 0x3000;
	fdaddr[4] = 0x4000;
	fdaddr[5] = 0x5000;
	fdaddr[6] = 0x6000;
	fdaddr[7] = 0x7000;
	fdaddr[8] = 0x8000;
	fdaddr[9] = 0x9000;
	fdaddr[10] = 0xa000;
	fdaddr[11] = 0xb000;
	fdaddr[12] = 0xc000;
	fdaddr[13] = 0xd000;
	fdaddr[14] = 0xe000;
	fdaddr[15] = 0xf000;

#ifdef HAS_DISKMANAGER
	extern void readDiskmap(char *);
	readDiskmap(dsk_path());
#endif
}
