/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2021 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * Emulation of an Intel Intellec iSBC 206 hark disk controller.
 *
 * Since I couldn't find any documentation on this controller, I looked at
 * Bill Beech's SIMH drivers
 * https://github.com/open-simh/simh/tree/master/Intel-Systems/common
 * and compared isbc202.c with isbc206.c, and they are nearly identical, so
 * I just started this from mds-isbc202.c.
 * I also gathered vital information from Mark Ogden's ISIS reverse
 * engineering: https://github.com/ogdenpm/intel80tools .
 *
 * History:
 * 08-JUN-2024 first version
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simmem.h"
#include "simio.h"

#ifdef HAS_ISBC206

#include "mds-isbc206.h"

#include "log.h"
static const char *TAG = "ISBC206";

#define ST_U0RDY	0x01	/* unit 0 ready */
#define ST_U1RDY	0x02	/* unit 1 ready */
#define ST_IPEND	0x04	/* interrupt pending */
#define ST_PRES		0x08	/* controller present */
#define ST_DD		0x10	/* double density present */
#define ST_HD		0x80	/* hard disk drive present */
#define ST_UNITS	(ST_U0RDY | ST_U1RDY)

				/* result types */
#define RT_IOERR	0x00	/* I/O complete error bits */
#define RT_DSKRD	0x01	/* hard drive ready status */

				/* I/O complete error bits */
#define IO_IDCMP	0x01	/* ID field miscompare */
#define IO_CRC		0x02	/* CRC error */
#define IO_SEEK		0x04	/* seek error */
#define IO_ADDR		0x08	/* address error */
#define IO_OURUN	0x10	/* data overrun/underrun */
#define IO_WPROT	0x20	/* write protect */
#define IO_WERR		0x40	/* write error */
#define IO_NRDY		0x80	/* not ready */

				/* hard drive ready status bits */
#define DR_UNIT0	0x40	/* unit 0 ready */
#define DR_UNIT1	0x80	/* unit 1 ready */

				/* channel word definitions */
#define CW_DWLEN	0x08	/* data word length (0 = 8, 1 = 16) */
#define CW_ICMSK	0x30	/* interrupt control mask */
#define CW_IEN		0x00	/* interrupt on completion/error */
#define CW_IDIS		0x10	/* all I/O interrupts disabled */
#define CW_RFS		0x40	/* random format sequence */

				/* hard drive instruction */
#define DI_OPMSK	0x07	/* op code mask */
#define DI_HEAD		0x08	/* head */
#define DI_USMSK	0x70	/* unit select mask */
#define DI_USSHF	4	/* unit select shift */
#define DI_UNIT0	0x40	/* drive 0 fixed */
#define DI_UNIT1	0x00	/* drive 0 removable */
#define DI_UNIT2	0x50	/* drive 1 fixed */
#define DI_UNIT3	0x10	/* drive 1 removable */

				/* hard drive operations */
#define OP_NOP		0x00	/* no operation */
#define OP_SEEK		0x01	/* seek */
#define OP_FMT		0x02	/* format track */
#define OP_REC		0x03	/* recalibrate */
#define OP_READ		0x04	/* read data */
#define OP_VERF		0x05	/* verify CRC */
#define OP_WT		0x06	/* write data */
#define OP_WDEL		0x07	/* write 'deleted' data */

/* 14" hard drives, really 400 tracks, 2 heads, 36 sectors per track */
#define SEC_SZ		128
#define SPT		144
#define TRK		200
#define DISK_SIZE	(TRK * SPT * SEC_SZ)

#define ISBC206_IRQ	2

static WORD iopb_addr;		/* address of I/O parameter block */
static BYTE status;		/* status byte */
static BYTE res_type;		/* result type */
static BYTE ioerr;		/* I/O complete error bits */
static char fndir[MAX_LFN];	/* directory path for disk image */
static char fn[MAX_LFN];	/* path/filename for disk image */
static int fd;			/* fd for disk file i/o */
static BYTE buf[SEC_SZ];	/* buffer for one sector */
static pthread_mutex_t status_mutex = PTHREAD_MUTEX_INITIALIZER;

/* these are our disk drives */
static const char *disks[4] = {
	"drivei.dsk",
	"drivej.dsk",
	"drivek.dsk",
	"drivel.dsk"
};

/* unit ready status bits */
static BYTE uready[4] = { ST_U0RDY, ST_U0RDY, ST_U1RDY, ST_U1RDY };

/* disk instruction unit to drive map */
static BYTE umap[8] = { 1, 3, 255, 255, 0, 2, 255, 255 };

/*
 * find and set path for disk images
 */
static void dsk_path(void)
{
	struct stat sbuf;

	/* if option -d is used disks are there */
	if (diskdir != NULL) {
		strcpy(fndir, diskd);
	} else {
		/* if not first try ./disks */
		if ((stat("./disks", &sbuf) == 0) && S_ISDIR(sbuf.st_mode)) {
			strcpy(fndir, "./disks");
		} else {
			/* nope, then DISKSDIR as set in Makefile */
			strcpy(fndir, DISKSDIR);
		}
	}
	strcat(fndir, "/");
}

BYTE isbc206_status_in(void)
{
	return status;
}

BYTE isbc206_res_type_in(void)
{
	pthread_mutex_lock(&status_mutex);
	status &= ~(ST_IPEND | ST_HD);
	pthread_mutex_unlock(&status_mutex);
	int_cancel(ISBC206_IRQ);
	return res_type;
}

BYTE isbc206_res_byte_in(void)
{
	BYTE data;

	if (res_type == RT_DSKRD) {
		data = 0;
		if (status & ST_U0RDY)
			data |= DR_UNIT0;
		if (status & ST_U1RDY)
			data |= DR_UNIT1;
	} else {
		data = ioerr;
		res_type = RT_DSKRD;
	}
	return data;
}

void isbc206_iopbl_out(BYTE data)
{
	iopb_addr = data;
}

void isbc206_iopbh_out(BYTE data)
{
	BYTE iocw, ioins, nsec, taddr, saddr, sec, b;
	WORD addr, track;
	int i, drive, op;
	off_t pos;
	struct stat s;

	iopb_addr |= data << 8;

	res_type = RT_IOERR;
	ioerr = 0;

	/* DMA read I/O parameter block from memory */
	iocw = dma_read(iopb_addr);
	if (iocw & CW_DWLEN) {
		LOGW(TAG, "control data word length is not 8-bit");
	}

	ioins = dma_read(iopb_addr + 1);

	op = ioins & DI_OPMSK;
	drive = umap[(ioins & DI_USMSK) >> DI_USSHF];
	if (drive == 255) {
		ioerr = IO_NRDY;
		goto done;
	}

	if ((status & uready[drive]) == 0) {
		ioerr = IO_NRDY;
		goto done;
	}

	nsec = dma_read(iopb_addr + 2);
	taddr = dma_read(iopb_addr + 3);
	saddr = dma_read(iopb_addr + 4);
	addr = dma_read(iopb_addr + 5);
	addr |= dma_read(iopb_addr + 6) << 8;

	/* undo ISIS logical to physical mapping */
	track = taddr;
	if (saddr & 0x80) {
		track += 256;
		saddr &= 0x7f;
	}
	if (!(ioins & DI_HEAD))
		saddr += 36;
	if (track & 1)
		saddr += 72;
	taddr = track >> 1;

	strcpy(fn, fndir);
	strcat(fn, disks[drive]);

	switch (op) {

	case OP_NOP:	/* no operation */
		break;

	case OP_SEEK:	/* seek */
		/* check track */
		if (taddr >= TRK) {
			ioerr = IO_ADDR;
			break;
		}
		break;

	case OP_FMT:	/* format */
		/* check track */
		if (taddr >= TRK) {
			ioerr = IO_ADDR;
			break;
		}

		/* unlink disk image */
		if (taddr == 0)
			unlink(fn);

		/* try to create new disk image */
		if ((fd = open(fn, O_RDWR | O_CREAT, 0644)) == -1) {
			ioerr = IO_NRDY;
			break;
		}

		/* expand new disk image to full size */
		if (taddr == 0 && ftruncate(fd, (off_t) DISK_SIZE) == -1) {
			ioerr = IO_NRDY;
			break;
		}

		if (iocw & CW_RFS) {	/* random format sequence */
			/*
			 * Reads a table of (sector, data) pairs from the IOPB
			 * buffer address and formats a track where the sectors
			 * are ordered in the table sequence and filled with
			 * the corresponding data value. In the disk image file
			 * the sectors are always in sequential order.
			 */
			for (i = 0; i < SPT / 4; i++) {

				/* get next (sector, data) pair and check it */
				sec = dma_read(addr++);
				if (sec == 0 || sec > SPT / 4) {
					ioerr = IO_ADDR;
					goto fdone;
				}
				b = dma_read(addr++);

				/* seek to sector */
				pos = (taddr * SPT + saddr + sec - 2) * SEC_SZ;
				if (lseek(fd, pos, SEEK_SET) == (off_t) -1) {
					ioerr = IO_SEEK;
					goto fdone;
				}

				/* fill buffer with data byte */
				memset(buf, b, SEC_SZ);

				/* write sector */
				if (write(fd, buf, SEC_SZ) != SEC_SZ) {
					ioerr = IO_OURUN;
					goto fdone;
				}
			}
		} else {
			/*
			 * Sequential track format, where the sectors are in
			 * order and filled with the data byte read from the
			 * IOPB buffer address.
			 */

			/* seek to track */
			pos = (taddr * SPT + saddr - 1) * SEC_SZ;
			if (lseek(fd, pos, SEEK_SET) == (off_t) -1) {
				ioerr = IO_SEEK;
				goto fdone;
			}

			/* fill buffer with data byte */
			b = dma_read(addr);
			memset(buf, b, SEC_SZ);

			/* write a track of sectors */
			for (i = 0; i < SPT / 4; i++) {
				if (write(fd, buf, SEC_SZ) != SEC_SZ) {
					ioerr = IO_OURUN;
					goto fdone;
				}
			}
		}

	fdone:
		close(fd);
		break;

	case OP_REC:	/* recalibrate */
		break;

	case OP_READ:	/* read data */
	case OP_VERF:	/* verify CRC, same as read with no memory write */

		/* check track/sector/nsec */
		if (taddr >= TRK || saddr == 0 || saddr > SPT || nsec > SPT
				 || saddr + nsec - 1 > SPT) {
			ioerr = IO_ADDR;
			break;
		}

		/* try to open disk image */
		if ((fd = open(fn, O_RDONLY)) == -1) {
			ioerr = IO_NRDY;
			break;
		}

		/* check for correct image size */
		if (fstat(fd, &s) == -1 || !S_ISREG(s.st_mode)
					|| s.st_size != DISK_SIZE) {
			ioerr = IO_NRDY;
			goto rdone;
		}

		/* seek to sector */
		pos = (taddr * SPT + saddr - 1) * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == (off_t) -1) {
			ioerr = IO_SEEK;
			goto rdone;
		}

		/* read the sectors */
		for (; nsec > 0; nsec--) {
			if (read(fd, buf, SEC_SZ) != SEC_SZ) {
				ioerr = IO_OURUN;
				goto rdone;
			}
			if (op == OP_READ) {
				for (i = 0; i < SEC_SZ; i++)
					dma_write(addr++, buf[i]);
			}
		}

	rdone:
		close(fd);
		break;

	case OP_WT:	/* write data */
	case OP_WDEL:	/* write 'deleted' data */

		/* check track/sector/nsec */
		if (taddr >= TRK || saddr == 0 || saddr > SPT || nsec > SPT
				 || saddr + nsec > SPT + 1) {
			ioerr = IO_ADDR;
			break;
		}

		/* try to open disk image */
		if ((fd = open(fn, O_RDWR)) == -1) {
			if ((fd = open(fn, O_RDONLY)) != -1) {
				ioerr = IO_WPROT;
				goto wdone;
			} else {
				ioerr = IO_NRDY;
				break;
			}
		}

		/* check for correct image size */
		if (fstat(fd, &s) == -1 || !S_ISREG(s.st_mode)
					|| s.st_size != DISK_SIZE) {
			ioerr = IO_NRDY;
			goto wdone;
		}

		/* seek to sector */
		pos = (taddr * SPT + saddr - 1) * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == (off_t) -1) {
			ioerr = IO_SEEK;
			goto wdone;
		}

		/* write sectors */
		for (; nsec > 0; nsec--) {
			for (i = 0; i < SEC_SZ; i++)
				buf[i] = dma_read(addr++);
			if (write(fd, buf, SEC_SZ) != SEC_SZ) {
				ioerr = IO_OURUN;
				goto wdone;
			}
		}

	wdone:
		close(fd);
		break;

	default:
		break;

	}

done:
	if ((iocw & CW_ICMSK) == CW_IEN) {
		pthread_mutex_lock(&status_mutex);
		status |= ST_IPEND;
		pthread_mutex_unlock(&status_mutex);
		int_request(ISBC206_IRQ);
	}
}

void isbc206_reset_out(BYTE data)
{
	UNUSED(data);

	isbc206_reset();
}

void isbc206_disk_check(void)
{
	int i;
	BYTE nstatus;
	char *pfn;
	struct stat s;
	static char fn_[MAX_LFN];

	if (!(status & ST_PRES) || res_type == RT_IOERR)
		return;

	nstatus = ST_UNITS;

	/* check disk ready status */
	strcpy(fn_, fndir);
	pfn = fn_ + strlen(fn_);
	for (i = 0; i <= 3; i++) {
		strcpy(pfn, disks[i]);
		if (stat(fn_, &s) == -1 || !S_ISREG(s.st_mode))
			nstatus &= ~uready[i];
	}

	if ((status & ST_UNITS) != nstatus) {
		pthread_mutex_lock(&status_mutex);
		status &= ~ST_UNITS;
		status |= nstatus | ST_IPEND;
		pthread_mutex_unlock(&status_mutex);
		int_request(ISBC206_IRQ);
	}
}

void isbc206_reset(void)
{
	int i;
	char *pfn;
	BYTE nstatus;
	struct stat s;

	/* set disk directory path */
	dsk_path();

	res_type = RT_DSKRD;
	ioerr = 0;

	if (getenv("ISBC206_OFF")) {
		status = 0;
		return;
	}

	nstatus = ST_PRES | ST_HD | ST_UNITS;

	/* check disk ready status */
	strcpy(fn, fndir);
	pfn = fn + strlen(fn);
	for (i = 0; i <= 3; i++) {
		strcpy(pfn, disks[i]);
		if (stat(fn, &s) == -1 || !S_ISREG(s.st_mode))
			nstatus &= ~uready[i];
	}

	status = nstatus;
}

#endif /* HAS_ISBC206 */
