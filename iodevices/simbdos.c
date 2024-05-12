/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * This module provides host file I/O using a BDOS-like interface.
 * Presently, functionality is limited to those features required
 * to move files between the host and CP/M file systems.
 *
 * Use OPENF to open a host file for reading. Use MAKEF to create
 * and open a host file for writing. CLOSEF should be called when
 * file I/O is completed even if a file was only opened for reading.
 *
 * All requests come in through the host_bdos_out function. This
 * function is called via an OUT instruction mapped in iosim.c.
 *
 * Files written to the host are treated as binary files unless
 * the file extension is found in the "textExts" table below. A
 * binary file is always written as 128 byte sectors all the way
 * through the last sector of the CP/M file. A text file is written
 * as 128 byte sectors until an EOF character (0x1A, ctrl-z) is found.
 * This results in the last write typically being less than 128 bytes.
 *
 * The CP/M programs R.COM and W.COM use this interface to transfer
 * files between the host and CP/M file systems. Note that on a
 * case-sensitive operating system, file names must be in all caps
 * as used in CP/M.
 *
 * History:
 * 03-OCT-2019 (Mike Douglas) Original
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "sim.h"
#include "simglb.h"
#ifdef FRONTPANEL
#include "frontpanel.h"
#endif
#include "memsim.h"

/* BDOS Equates */

#define	OPENF	15		/* open file */
#define	CLOSEF	16		/* close file */
#define READF	20		/* read file */
#define WRITEF	21		/* write file */
#define MAKEF	22		/* make file */
#define SETDMA	26		/* set DMA address */

#define	SECLEN	128		/* logical sector length */
#define	CTRL_Z	0x1A		/* CP/M EOF character */

/* The following file types will be treated as text files */

static const char *textExts[] = {
	"asm", "mac", "src", "txt", "prn", "lst",
	"sym", "lib", "c",   "pas", "for", "rat",
	"cbl", "plm", "pli", "hex", "sub", NULL
};

/* Static variables */

static FILE *fp = NULL;		/* file pointer */
static WORD dmaAddr = 0x80;	/* buffer address in emulated space */
static int textFile = 0;	/* text file flag */

/*
 * host_bdos_out - an output to this I/O device is somewhat equivalent
 *    to calling the CP/M BDOS entry point, but the file I/O operation
 *    is performed on the host file system. As a bit of protection from
 *    inadvertent entry, the output value must be the compliment of the
 *    command in register C.
 */

void host_bdos_out(BYTE outByte)
{
	WORD fcbAddr;		/* address of FCB in simulator memory */
	char fname[16];
	char extension[8];
	BYTE buf[128];
	char openFlags[4];	/* flags for fopen call */
	int xferLen;
	int i;

	outByte = ~outByte;	/* compiler requires assignment */
	if (outByte != C)	/*    for the compare to work */
		return;

	/* OPEN or MAKE file */

	A = 0xff;		/* assume error status */
	fcbAddr = (D << 8) + E;	/* FCB address in simulator memory */

	if ((C == OPENF) || (C == MAKEF)) {
		for (i = 0; i < 8; i++) {	/* copy file name */
			fname[i] = tolower(memrdr(fcbAddr + 1 + i));
			if (fname[i] == ' ')
				break;
		}
		fname[i] = 0;

		for (i = 0; i < 3; i++) {	/* copy extension */
			extension[i] = tolower(memrdr(fcbAddr + 9 + i));
			if (extension[i] == ' ')
				break;
		}
		extension[i] = 0;

		if (extension[0] != 0) {	/* form full file name */
			strcat(fname, ".");
			strcat(fname, extension);
		}

		if (C == MAKEF) {
			strcpy(openFlags, "wb"); /* MAKEF opens for writing */

#ifdef SIMBDOS_NO_OVERWRITE
			if (fopen(fname, "rb") != NULL) { /* file exist? */
				fclose(fp);
				openFlags[0] = 0; /* yes, don't over-write */
			}
#endif
			textFile = 0;		/* binary file is assumed */
			for (i = 0; textExts[i] != NULL; i++)
				if (strcmp(extension, textExts[i]) == 0) {
					textFile = 1;	/* it's a text file */
					break;
				}
		} else
			strcpy(openFlags, "rb"); /* OPENF for reading */

		if (openFlags[0] != 0)		/* don't open if null string */
			if ((fp = fopen(fname, openFlags)) != NULL)
				A = 0;		/* success */
	}

	/* CLOSE file */

	else if (C == CLOSEF) {
		if (fp != NULL)
			fclose(fp);
		A = 0;
	}

	/* READ file */

	else if (C == READF) {
		if (fp != NULL) {
			xferLen = fread(buf, 1, SECLEN, fp);
			if (xferLen != 0) {
				for (i = 0; i < xferLen; i++)
					memwrt(dmaAddr + i, buf[i]);
				for (; i < SECLEN; i++)
					memwrt(dmaAddr + i, CTRL_Z);
				A = 0;
			}
		}
	}

	/* WRITE file */

	else if (C == WRITEF) {
		if (fp != NULL) {
			for (xferLen = 0; xferLen < SECLEN; xferLen++) {
				buf[xferLen] = memrdr(dmaAddr + xferLen);
				if ((buf[xferLen] == CTRL_Z) && textFile)
					break;	/* ctrl-z (EOF) found */
			}
			if (xferLen == 0)	/* record contains only ctrl-z */
				A = 0;
			else if ((size_t) xferLen == fwrite(buf, 1, xferLen, fp))
				A = 0;
		}
	}

	/* Set DMA Address */

	else if (C == SETDMA) {
		dmaAddr = fcbAddr;
		A = 0;
	}
}
