/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2021 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * Emulation of an Intel Intellec iSBC 202 double density disk controller
 *
 * History:
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "sim.h"
#include "simglb.h"
#include "memsim.h"
#include "mds-isbc202.h"
#include "log.h"

				/* status input bits */
#define ST_U0RDY	0x01	/* unit 0 ready */
#define ST_U1RDY	0x02	/* unit 1 ready */
#define ST_IPEND	0x04	/* interrupt pending */
#define ST_PRES		0x08	/* controller present */
#define ST_DD		0x10	/* double density present */
#define ST_U2RDY	0x20	/* unit 2 ready */
#define ST_U3RDY	0x40	/* unit 3 ready */

				/* result types */
#define RT_IOERR	0x00	/* I/O complete error bits */
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
#define DR_UNIT2	0x10	/* unit 2 ready */
#define DR_UNIT3	0x20	/* unit 3 ready */
#define DR_UNIT0	0x40	/* unit 0 ready */
#define DR_UNIT1	0x80	/* unit 1 ready */

				/* channel word definitions */
#define CW_DWLEN	0x08	/* data word length (0 = 8, 1 = 16) */
#define CW_ICMSK	0x30	/* interrupt control mask */
#define CW_IEN		0x00	/* interrupt on completion/error */
#define CW_IDIS		0x10	/* all I/O interrupts disabled */
#define CW_RFS		0x40	/* random format sequence */

				/* diskette instruction */
#define DI_OPMSK	0x07	/* op code mask */
#define DI_DWLEN	0x08	/* data word length (0 = 8, 1 = 16) */
#define DI_USMSK	0x30	/* unit select mask */
#define DI_USSHF	4	/* unit select shift */
#define DI_UNIT0	0x00	/* drive 0 */
#define DI_UNIT1	0x10	/* drive 1 */
#define DI_UNIT2	0x20	/* drive 2 */
#define DI_UNIT3	0x30	/* drive 3 */

				/* diskette operations */
#define OP_NOP		0x00	/* no operation */
#define OP_SEEK		0x01	/* seek */
#define OP_FMT		0x02	/* format track */
#define OP_REC		0x03	/* recalibrate */
#define OP_READ		0x04	/* read data */
#define OP_VERF		0x05	/* verify CRC */
#define OP_WT		0x06	/* write data */
#define OP_WDEL		0x07	/* write 'deleted' data */

/* 8" M2FM double density disks */
#define SEC_SZ		128
#define SPT		52
#define TRK		77
#define DISK_SIZE	(TRK * SPT * SEC_SZ)

#define ISBC202_IRQ	2

static const char *TAG = "ISBC202";

static WORD iopb_addr;		/* address of I/O parameter block */
static BYTE status;		/* status byte */
static int rdy_change;		/* unit readiness change flag */
static int res_type;		/* result type */
static BYTE io_comperr;		/* I/O complete error bits */
static char fn[MAX_LFN];	/* path/filename for disk image */
static int fd;			/* fd for disk file i/o */
static BYTE buf[SEC_SZ];	/* buffer for one sector */

/* these are our disk drives */
static const char *disks[4] = {
	"drivea.dsk",
	"driveb.dsk",
	"drivec.dsk",
	"drived.dsk"
};

/*
 * find and set path for disk images
 */
void dsk_path(void)
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
	}
}

BYTE isbc202_status_in(void)
{
	return (status);
}

BYTE isbc202_res_type_in(void)
{
	extern void int_cancel(int);

	res_type = rdy_change ? RT_DSKRD : RT_IOERR;
	status &= ~ST_IPEND;
	int_cancel(ISBC202_IRQ);
	return (res_type);
}

BYTE isbc202_res_byte_in(void)
{
	BYTE data;

	if (res_type == RT_DSKRD) {
		data = 0;
		if (status & ST_U0RDY)
			data |= DR_UNIT0;
		if (status & ST_U1RDY)
			data |= DR_UNIT1;
		if (status & ST_U2RDY)
			data |= DR_UNIT2;
		if (status & ST_U3RDY)
			data |= DR_UNIT3;
	} else
		data = io_comperr;
	return (data);
}

void isbc202_iopbl_out(BYTE data)
{
	iopb_addr = data;
}

void isbc202_iopbh_out(BYTE data)
{
	extern void int_request(int);

	BYTE iocw, ioins, nsec, taddr, saddr, b;
	WORD addr;
	int i;
	off_t pos;
	struct stat s;

	iopb_addr |= data << 8;

	io_comperr = 0;

	/* DMA read I/O parameter block from memory */
	iocw = dma_read(iopb_addr);
	if (iocw & CW_DWLEN) {
		LOGW(TAG, "data word length is not 8-bit");
	}
	ioins = dma_read(iopb_addr + 1);
	if (ioins & DI_DWLEN) {
		LOGW(TAG, "data word length is not 8-bit");
	}
	nsec = dma_read(iopb_addr + 2);
	taddr = dma_read(iopb_addr + 3);
	saddr = dma_read(iopb_addr + 4);
	addr = dma_read(iopb_addr + 5);
	addr |= dma_read(iopb_addr + 6) << 8;

	switch (ioins & DI_OPMSK) {

	case OP_NOP:	/* no operation */
		break;

	case OP_SEEK:	/* seek */
		/* check track */
		if (taddr >= TRK) {
			io_comperr = IO_ADDR;
			break;
		}
		break;

	case OP_FMT:	/* format */
		/* check track */
		if (taddr >= TRK) {
			io_comperr = IO_ADDR;
			break;
		}

		/* unlink disk image */
		dsk_path();
		strcat(fn, "/");
		strcat(fn, disks[(ioins & DI_USMSK) >> DI_USSHF]);
		if (taddr == 0)
			unlink(fn);

		/* try to create new disk image */
		if ((fd = open(fn, O_RDWR | O_CREAT, 0644)) == -1) {
			io_comperr = IO_NRDY;
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

				/* get next (sector, data) pair and check sector */
				saddr = dma_read(addr++);
				if (saddr == 0 || saddr > SPT) {
					io_comperr = IO_ADDR;
					goto fdone;
				}
				b = dma_read(addr++);

				/* seek to sector */
				pos = (taddr * SPT + saddr - 1) * SEC_SZ;
				if (lseek(fd, pos, SEEK_SET) == (off_t) -1) {
					io_comperr = IO_SEEK;
					goto fdone;
				}

				/* fill buffer with data byte */
				memset(buf, b, SEC_SZ);

				/* write sector */
				if (write(fd, buf, SEC_SZ) != SEC_SZ) {
					io_comperr = IO_OURUN;
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
				io_comperr = IO_SEEK;
				goto fdone;
			}

			/* fill buffer with data byte */
			b = dma_read(addr);
			memset(buf, b, SEC_SZ);

			/* write a track of sectors */
			for (i = 0; i < SPT; i++) {
				if (write(fd, buf, SEC_SZ) != SEC_SZ) {
					io_comperr = IO_OURUN;
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
			io_comperr = IO_ADDR;
			break;
		}

		/* try to open disk image */
		dsk_path();
		strcat(fn, "/");
		strcat(fn, disks[(ioins & DI_USMSK) >> DI_USSHF]);
		if ((fd = open(fn, O_RDONLY)) == -1) {
			io_comperr = IO_NRDY;
			break;
		}

		/* check for correct image size */
		if (fstat(fd, &s) == -1 || s.st_size != DISK_SIZE) {
			io_comperr = IO_NRDY;
			goto rdone;
		}

		/* seek to sector */
		pos = (taddr * SPT + saddr - 1) * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == (off_t) -1) {
			io_comperr = IO_SEEK;
			goto rdone;
		}

		/* read the sectors */
		for (; nsec > 0; nsec--) {
			if (read(fd, buf, SEC_SZ) != SEC_SZ) {
				io_comperr = IO_OURUN;
				goto rdone;
			}
			if ((ioins & DI_OPMSK) == OP_READ) {
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
			io_comperr = IO_ADDR;
			break;
		}

		/* try to open disk image */
		dsk_path();
		strcat(fn, "/");
		strcat(fn, disks[(ioins & DI_USMSK) >> DI_USSHF]);
		if ((fd = open(fn, O_RDWR)) == -1) {
			if ((fd = open(fn, O_RDONLY)) != -1) {
				io_comperr = IO_WPROT;
				goto wdone;
			} else {
				io_comperr = IO_NRDY;
				break;
			}
		}

		/* check for correct image size */
		if (fstat(fd, &s) == -1 || s.st_size != DISK_SIZE) {
			io_comperr = IO_NRDY;
			goto wdone;
		}

		/* seek to sector */
		pos = (taddr * SPT + saddr - 1) * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == (off_t) -1) {
			io_comperr = IO_SEEK;
			goto wdone;
		}

		/* write sectors */
		for (; nsec > 0; nsec--) {
			for (i = 0; i < SEC_SZ; i++)
				buf[i] = dma_read(addr++);
			if (write(fd, buf, SEC_SZ) != SEC_SZ) {
				io_comperr = IO_OURUN;
				goto wdone;
			}
		}

	wdone:
		close(fd);
		break;

	default:
		break;

	}

	status |= ST_IPEND;
	if ((iocw & CW_ICMSK) == CW_IEN)
		int_request(ISBC202_IRQ);
}

void isbc202_reset_out(BYTE data)
{
	UNUSED(data);

	isbc202_reset();
}

void isbc202_reset(void)
{
	status = ST_PRES | ST_DD | ST_U0RDY | ST_U1RDY | ST_U2RDY | ST_U3RDY;
	rdy_change = 0;
	io_comperr = 0;
}
