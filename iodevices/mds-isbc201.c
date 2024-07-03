/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2021 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * Emulation of an Intel Intellec iSBC 201 single density disk controller
 *
 * History:
 * 09-JUN-2024 first version
 */

#include <stddef.h>
#include <stdio.h>
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

#ifdef HAS_ISBC201

#include "mds-isbc201.h"

#include "log.h"
static const char *TAG = "ISBC201";

				/* status input bits */
#define ST_U0RDY	0x01	/* unit 0 ready */
#define ST_U1RDY	0x02	/* unit 1 ready */
#define ST_IPEND	0x04	/* interrupt pending */
#define ST_PRES		0x08	/* controller present */
#define ST_UNITS	(ST_U0RDY | ST_U1RDY)

				/* result types */
#define RT_IOERR	0x00	/* I/O complete error bits */
#define RT_IOERRL	0x01	/* I/O complete error bits (linked),
				   top 6 bits are block number */
#define RT_DSKRD	0x02	/* diskette ready status */

				/* I/O complete error bits */
#define IO_DELR		0x01	/* deleted record */
#define IO_CRC		0x02	/* CRC error */
#define IO_SEEK		0x04	/* seek error */
#define IO_ADDR		0x08	/* address error */
#define IO_OURUN	0x10	/* data overrun/underrun */
#define IO_WPROT	0x20	/* write protect */
#define IO_WERR		0x40	/* write error */
#define IO_NRDY		0x80	/* not ready */

				/* diskette ready status bits */
#define DR_UNIT0	0x40	/* unit 0 ready */
#define DR_UNIT1	0x80	/* unit 1 ready */

				/* channel word definitions */
#define CW_WAIT		0x01	/* wait */
#define CW_BRCHW	0x02	/* branch on wait */
#define CW_SUCC		0x04	/* successor */
#define CW_DWLEN	0x08	/* data word length (0 = 8, 1 = 16) */
#define CW_ICMSK	0x30	/* interrupt control mask */
#define CW_IEN		0x00	/* interrupt on completion/error */
#define CW_IDIS		0x10	/* all I/O interrupts disabled */
#define CW_RFS		0x40	/* random format sequence */
#define CW_LCKOV	0x80	/* lock override */

				/* diskette instruction */
#define DI_OPMSK	0x07	/* op code mask */
#define DI_DWLEN	0x08	/* data word length (0 = 8, 1 = 16) */
#define DI_USMSK	0x30	/* unit select mask */
#define DI_USSHF	4	/* unit select shift */
#define DI_UNIT0	0x00	/* drive 0 */
#define DI_UNIT1	0x30	/* drive 1 */

				/* diskette operations */
#define OP_NOP		0x00	/* no operation */
#define OP_SEEK		0x01	/* seek */
#define OP_FMT		0x02	/* format track */
#define OP_REC		0x03	/* recalibrate */
#define OP_READ		0x04	/* read data */
#define OP_VERF		0x05	/* verify CRC */
#define OP_WT		0x06	/* write data */
#define OP_WDEL		0x07	/* write 'deleted' data */

/* 8" single density disks */
#define SEC_SZ		128
#define SPT		26
#define TRK		77
#define DISK_SIZE	(TRK * SPT * SEC_SZ)

#define ISBC201_IRQ	2

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
static const char *disks[2] = {
	"drivee.dsk",
	"drivef.dsk",
};

/* unit ready status bits */
static BYTE uready[2] = { ST_U0RDY, ST_U1RDY };

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

BYTE isbc201_status_in(void)
{
	return status;
}

BYTE isbc201_res_type_in(void)
{
	pthread_mutex_lock(&status_mutex);
	status &= ~ST_IPEND;
	pthread_mutex_unlock(&status_mutex);
	int_cancel(ISBC201_IRQ);
	return res_type;
}

BYTE isbc201_res_byte_in(void)
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

void isbc201_iopbl_out(BYTE data)
{
	iopb_addr = data;
}

void isbc201_iopbh_out(BYTE data)
{
	BYTE iocw, ioins, nsec, taddr, saddr, b;
	WORD addr;
#if 0
	BYTE blknr;
	WORD niopb;
#endif
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
	if (iocw & (CW_SUCC | CW_BRCHW | CW_WAIT)) {
		LOGW(TAG, "unsupported channel word bits set 0x%02x", iocw);
	}

	ioins = dma_read(iopb_addr + 1);
	if (ioins & DI_DWLEN) {
		LOGW(TAG, "instruction data word length is not 8-bit");
	}

	op = ioins & DI_OPMSK;
	drive = ((ioins & DI_USMSK) >> DI_USSHF) & 1;
	if ((status & uready[drive]) == 0) {
		ioerr = IO_NRDY;
		goto done;
	}

	nsec = dma_read(iopb_addr + 2);
	taddr = dma_read(iopb_addr + 3);
	saddr = dma_read(iopb_addr + 4) & 0x1f; /* without bank/fddnum */
	addr = dma_read(iopb_addr + 5);
	addr |= dma_read(iopb_addr + 6) << 8;
#if 0
	blknr = dma_read(iopb_addr + 7);
	niopb = dma_read(iopb_addr + 8);
	niopb |= dma_read(iopb_addr + 9) << 8;
#endif

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
			for (i = 0; i < SPT; i++) {

				/* get next (sector, data) pair and check it */
				saddr = dma_read(addr++);
				if (saddr == 0 || saddr > SPT) {
					ioerr = IO_ADDR;
					goto fdone;
				}
				b = dma_read(addr++);

				/* seek to sector */
				pos = (taddr * SPT + saddr - 1) * SEC_SZ;
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
			pos = taddr * SPT * SEC_SZ;
			if (lseek(fd, pos, SEEK_SET) == (off_t) -1) {
				ioerr = IO_SEEK;
				goto fdone;
			}

			/* fill buffer with data byte */
			b = dma_read(addr);
			memset(buf, b, SEC_SZ);

			/* write a track of sectors */
			for (i = 0; i < SPT; i++) {
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
		int_request(ISBC201_IRQ);
	}
}

void isbc201_reset_out(BYTE data)
{
	UNUSED(data);

	isbc201_reset();
}

void isbc201_disk_check(void)
{
	int i;
	BYTE nstatus;
	char *pfn;
	struct stat s;
	static char fn_[MAX_LFN];

	if (res_type == RT_IOERR)
		return;

	/* check disk ready status */
	strcpy(fn_, fndir);
	pfn = fn_ + strlen(fn_);
	nstatus = ST_UNITS;
	for (i = 0; i <= 1; i++) {
		strcpy(pfn, disks[i]);
		if (stat(fn_, &s) == -1 || !S_ISREG(s.st_mode))
			nstatus &= ~uready[i];
	}

	if ((status & ST_UNITS) != nstatus) {
		pthread_mutex_lock(&status_mutex);
		status &= ~ST_UNITS;
		status |= nstatus | ST_IPEND;
		pthread_mutex_unlock(&status_mutex);
		int_request(ISBC201_IRQ);
	}
}

void isbc201_reset(void)
{
	int i;
	char *pfn;
	BYTE nstatus;
	struct stat s;

	/* set disk directory path */
	dsk_path();

	res_type = RT_DSKRD;
	ioerr = 0;

	/* check disk ready status */
	strcpy(fn, fndir);
	pfn = fn + strlen(fn);
	nstatus = ST_UNITS;
	for (i = 0; i <= 1; i++) {
		strcpy(pfn, disks[i]);
		if (stat(fn, &s) == -1 || !S_ISREG(s.st_mode))
			nstatus &= ~uready[i];
	}
	if (nstatus)
		nstatus |= ST_PRES;

	status = nstatus;
}

#endif /* HAS_ISBC201 */
