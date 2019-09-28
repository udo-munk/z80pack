/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * Emulation of a Mostek FLP-80 Floppy Disk Controller
 *		(a WD1771 based FDC)
 *
 * History:
 * 16-SEP-2019 (Mike Douglas) created from tarbell-fdc.c
 * 28-SEP-2019 (Udo Munk) use logging
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sim.h"
#include "simglb.h"
#include "log.h"

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

/* Floppy board status bit masks */

#define sINTERRUPT	0x02	/* command is finished when set */
#define	sINPUT_READY	0x04	/* data is available in FIFO */
#define	sOUTPUT_READY	0x08	/* FIFO ready to accept data */

/* Floppy board control bit masks */

#define	cDRIVE0		0x01	/* drive select 1 */
#define	cDRIVE1		0x02	/* drive select 2 */
#define	cDRIVE2		0x04	/* drive select 3 */
#define cDRIVE3		0x08	/* drive select 4 */
#define	cFIFO_RESET	0x20	/* reset FIFO when toggled 0-1-0 */
#define cFIFO_BUFFERING	0x40	/* 1=data thru FIFO, 0=direct to 1771 */
#define	cFIFO_WRITE	0x80	/* 1=processor writing to FIFO */

/* WD1771 status and command bit masks */

#define	sTRACK0		0x04	/* drive on track 0 (type I) */
#define	sCRC_ERROR	0x08	/* used as error for all types */
#define	sRECORD_NOT_FOUND 0x10	/* record not found (type II) */
#define	sSEEK_ERROR	0x10	/* seek error (type I) */
#define sHEAD_LOADED	0x20	/* head is loaded (type I) */
#define	sWRITE_FAULT	0x20	/* write fault (type II) */
#define	sWRITE_PROTECT	0x40	/* write protect (type I) */
#define	sNOT_READY	0x80	/* drive not ready (all) */

#define	cUPDATE_TRACK	0x10	/* STEP updates track register */

/*  Static variable definitions  */

static const char *TAG = "FLP-80";

static BYTE fdc_stat;		/* status register */
static BYTE fdc_track;		/* track register */
static BYTE fdc_sec;		/* sector register */
static BYTE fdc_data;		/* data register */
static BYTE board_stat;		/* FLP80 board status register */
static BYTE board_ctl;		/* FLP80 board control register */
static int disk;		/* current disk # */
static int state;		/* fdc state */
static char fn[256];		/* path/filename for disk image */
static int fd;			/* fd for disk file i/o */
static int dcnt;		/* data counter read/write */
static BYTE buf[SEC_SZ];	/* buffer for one sector */

/*
 * get_disk_filename
 *     Returns filename of the current disk (disk) by searching the
 *     config file. Filename is placed in static variable fn;
 */
void get_disk_filename(void)
{
	FILE *fp;
	char inbuf[256];
	char *left, *right;

	*fn = (char)0;		/* init to null string */	

	if ((fp = fopen("conf/config.txt", "r")) != NULL) {

		while (fgets(inbuf, 256, fp) != NULL) {
			if ((inbuf[0]=='\n') || (inbuf[0]=='\r') ||
			    (inbuf[0]=='#'))
				continue;
			left = strtok(inbuf, "= \t\r");
			if (0 == strncmp(left, "drive", 5)) {
				if (left[5] == (char)disk + '0') { /* match drive? */
					right = strtok(NULL, "= \t\r");
					strcpy(fn, right);
					fclose(fp);
					break;
				}
			}
		} 	/* end while */
	}
}

/*
 * FLP-80 board status port input
 */
BYTE fdcBoard_stat_in(void)
{
	return(board_stat);
}

/*
 * FLP-80 board control port input
 */
BYTE fdcBoard_ctl_in(void)
{
	return(board_ctl);
}

/*
 * FLP-80 board control port output
 *    The lower four bits of the control byte are the drive select
 *    bits. Convert bit positions 1,2,4,8 to value 0-3 in disk.
 *    Invalid bit combinations are forced to a disk value 0-3.
 */
void fdcBoard_ctl_out(BYTE data)
{
	board_ctl = data;
	disk = (data & 0x0f) >> 1;		/* 1->0, 2->1, 4->2, 8->4 */
	if (disk > 3) 				/* 8->4->3 */
		disk = (disk - 1) & 0x03;
}

/*
 * FDC 1771 status port
 */
BYTE fdc1771_stat_in(void)
{
	board_stat &= ~sINTERRUPT;		/* read clears INTRQ */
	return(fdc_stat);
}

/*
 * FDC 1771 command port
 */
void fdc1771_cmd_out(BYTE data)
{
	board_stat = sINTERRUPT;		/* assume INTRQ will be asserted */

	if ((data & 0xf0) == 0) {		/* restore (seek track 0) */
		fdc_track = 0;
		fdc_stat = sTRACK0;		/* track 0 flag true */

	} else if ((data & 0xf0) == 0x10) {	/* seek */
		if (fdc_data < TRK) {
			fdc_track = fdc_data;
			fdc_stat = 0;
		}
		else
			fdc_stat = sSEEK_ERROR;	/* assumes V bit set */

		if (fdc_track == 0)		/* update track zero flag */
			fdc_stat |= sTRACK0;

	} else if ((data & 0xe0) == 0x20) {	/* step same direction */
		printf("Mostek FLP-80: step not implemented\r\n");
		fdc_stat = sSEEK_ERROR;
		
		if (fdc_track == 0)		/* check to set track 0 flag */
			fdc_stat |= sTRACK0;
			
	} else if ((data & 0xe0) == 0x40) {	/* step in */
		if (data & cUPDATE_TRACK)	/* update track register? */
			fdc_track++;
		if (fdc_track < TRK)
			fdc_stat = 0;
		else
			fdc_stat = sSEEK_ERROR;	/* assumes V bit set */

		if (fdc_track == 0)		/* udpate track zero flag */	
			fdc_stat |= sTRACK0;

	} else if ((data & 0xe0) == 0x60) {	/* step out */
		if (data & cUPDATE_TRACK)	/* update track register? */
			fdc_track--;
		if (fdc_track < TRK)
			fdc_stat = 0;
		else
			fdc_stat = sSEEK_ERROR;	/* assumes V bit set */

		if (fdc_track == 0)		/* update track zero flag */	
			fdc_stat |= sTRACK0;

	} else if ((data & 0xf0) == 0x80) {	/* read single sector */
		state = FDC_READ;
		dcnt = 0;
		fdc_stat = 0;
		board_stat = sINPUT_READY;	/* show ready to read data */

	} else if ((data & 0xf0) == 0x90) {	/* read multiple sector */
		LOGW(TAG, "read multiple sector not implemented");
		fdc_stat = sRECORD_NOT_FOUND;

	} else if ((data & 0xf0) == 0xa0) {	/* write single sector */
		state = FDC_WRITE;
		dcnt = 0;
		fdc_stat = 0;
		board_stat = sOUTPUT_READY;	/* show ready to accept data */

	} else if ((data & 0xf0) == 0xb0) {	/* write multiple sector */
		LOGW(TAG, "write multiple sector not implemented");
		fdc_stat = sRECORD_NOT_FOUND;

	} else if (data == 0xc4) {		/* read address */
		state = FDC_READADR;
		dcnt = 0;
		fdc_stat = 0;
		board_stat = sINPUT_READY;	/* show ready to read data */

	} else if ((data & 0xf0) == 0xe0) {	/* read track */
		LOGW(TAG, "read track not implemented");
		fdc_stat = sRECORD_NOT_FOUND;

	} else if ((data & 0xf0) == 0xf0) {	/* write track */
		state = FDC_WRTTRK;
		dcnt = 0;
		fdc_stat = 0;
		board_stat = sOUTPUT_READY;	/* show ready to accept data */

	} else if ((data & 0xf0) == 0xd0) {	/* force interrupt */
		state = FDC_IDLE;		/* abort any command */
		fdc_stat = 0;

	} else {
		LOGW(TAG, "unknown command, %02x", data);
		fdc_stat = sCRC_ERROR;
	}
}

/*
 * FDC 1771 track input port
 */
BYTE fdc1771_track_in(void)
{
	return(fdc_track);
}

/*
 * FDC 1771 track output port
 */
void fdc1771_track_out(BYTE data)
{
	fdc_track = data;
}

/*
 * FDC 1771 sector input port
 */
BYTE fdc1771_sec_in(void)
{
	return(fdc_sec);
}

/*
 * FDC 1771 sector output port
 */
void fdc1771_sec_out(BYTE data)
{
	fdc_sec = data;
}

/*
 * FDC 1771 data input port
 */
BYTE fdc1771_data_in(void)
{
	long pos;		/* seek position */

	switch (state) {
	case FDC_READ:		/* read data from disk sector */

		/* first byte? */
		if (dcnt == 0) {
			board_stat = sINTERRUPT; /* indicate command complete */

			/* check track/sector */
			if ((fdc_track >= TRK) || (fdc_sec > SPT)) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = sRECORD_NOT_FOUND;
				return((BYTE) 0);
			}

			/* check disk drive */
			if ((disk < 0) || (disk > 3)) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = sNOT_READY;
				return((BYTE) 0);
			}

			/* try to open disk image */
			get_disk_filename();
			if ((fd = open(fn, O_RDONLY)) == -1) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = sNOT_READY;
				return((BYTE) 0);
			}

			/* seek to sector */
			pos = (fdc_track * SPT + fdc_sec - 1) * SEC_SZ;
			if (lseek(fd, pos, SEEK_SET) == -1L) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = sRECORD_NOT_FOUND;
				close(fd);
				return((BYTE) 0);
			}

			/* read the sector */
			if (read(fd, &buf[0], SEC_SZ) != SEC_SZ) {
				state = FDC_IDLE;	/* abort read command */
				fdc_stat = sRECORD_NOT_FOUND;
				close(fd);
				return((BYTE) 0);
			}
			close(fd);
			board_stat = sINPUT_READY + sINTERRUPT;
		}

		/* last byte? */
		if (dcnt == SEC_SZ - 1) {
			state = FDC_IDLE;
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
			board_stat = sINPUT_READY + sINTERRUPT;
		}

		/* last byte? */
		if (dcnt == 5) {
			state = FDC_IDLE;
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
 * FDC 1771 data output port
 */
void fdc1771_data_out(BYTE data)
{
	long pos;			/* seek position */
	static int wrtstat;		/* state while formatting track */
	static int bcnt;		/* byte counter for sector data */
	static int secs;		/* # of sectors written so far */

	switch (state) {
	case FDC_WRITE:			/* write data to disk sector */
		if (dcnt == 0) {
			board_stat = sINTERRUPT; /* indicate command complete */

			/* check track/sector */
			if ((fdc_track >= TRK) || (fdc_sec > SPT)) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = sRECORD_NOT_FOUND;
				return;
			}

			/* check disk drive */
			if ((disk < 0) || (disk > 3)) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = sNOT_READY;
				return;
			}

			/* try to open disk image */
			get_disk_filename();
			if ((fd = open(fn, O_RDWR)) == -1) {
				if ((fd = open(fn, O_RDONLY)) != -1) {
					close(fd);
					fdc_stat = sWRITE_PROTECT;
				} else {
					fdc_stat = sNOT_READY;
				}
				state = FDC_IDLE;	/* abort command */
				return;
			}

			/* seek to sector */
			pos = (fdc_track * SPT + fdc_sec - 1) * SEC_SZ;
			if (lseek(fd, pos, SEEK_SET) == -1L) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = sRECORD_NOT_FOUND;
				close(fd);
				return;
			}
			board_stat = sOUTPUT_READY + sINTERRUPT;
		}

		/* write data bytes into sector buffer */
		buf[dcnt++] = data;

		/* last byte? */
		if (dcnt == SEC_SZ) {
			state = FDC_IDLE;
			if (write(fd, &buf[0], SEC_SZ) == SEC_SZ) 
				fdc_stat = 0;
			else
				fdc_stat = sWRITE_FAULT;
			close(fd);
		}
		break;

	case FDC_WRTTRK:		/* write (format) TRACK */
		if (dcnt == 0) {
			board_stat = sINTERRUPT; /* indicate command complete */

			/* unlink disk image */
			get_disk_filename();
			if (fdc_track == 0)
				unlink(fn);
			/* try to create new disk image */
			if ((fd = open(fn, O_RDWR|O_CREAT, 0644)) == -1) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = sNOT_READY;
				return;
			}
			/* seek to track */
			pos = fdc_track * SPT  * SEC_SZ;
			if (lseek(fd, pos, SEEK_SET) == -1L) {
				state = FDC_IDLE;	/* abort command */
				fdc_stat = sRECORD_NOT_FOUND;
				close(fd);
				return;
			}
			/* now wait for sector data */
			board_stat = sOUTPUT_READY + sINTERRUPT;
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
					fdc_stat = sWRITE_FAULT; 
				wrtstat = 1;
			}
		}
		/* all sectors of track written? */
		if (secs == SPT) {
			state = FDC_IDLE;
			close(fd);
		}
		break;

	default:			/* normally track # for seek */
		fdc_data = data;
		break;
	}
}

/*
 * Reset FDC
 */
void fdc_reset(void)
{
	fdc_stat = fdc_track = fdc_sec = fdc_data = disk = state = dcnt = 0;
	board_stat = board_ctl = 0;
}
