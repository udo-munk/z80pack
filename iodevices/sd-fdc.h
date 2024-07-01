/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2024, by Udo Munk
 *
 * This FDC does not emulate any existing hardware,
 * special not these with some LSI chip, it is designed
 * for bare metal with disk images on MicroSD and such.
 * It needs one single I/O port, send command to the port,
 * read result what it did, done. It does DMA transfer of
 * a complete sector. It has a rock solid state engine,
 * that can not get stuck by some software sending random
 * data, is all ignored. It is platform independend, the
 * platform must provide the low level functions, which
 * depend on what storage hardware actually is used.
 *
 * History:
 * 23-MAY-2024 implemented FDC, CP/M boot code & CBIOS
 */

#ifndef SD_FDC_INC
#define SD_FDC_INC

/* floppy disk definitions */
#define SEC_SZ		128	/* sector size */
#define SPT		26	/* sectors per track */
#define TRK		77	/* number of tracks */

/* FDC status codes */
#define FDC_STAT_OK	0	/* command executed successfull */
#define FDC_STAT_DISK	1	/* disk drive out of range */
#define FDC_STAT_NODISK	2	/* disk file open error */
#define FDC_STAT_SEEK	3	/* disk file seek error */
#define FDC_STAT_READ	4	/* disk file read error */
#define FDC_STAT_WRITE	5	/* disk file write error */
#define FDC_STAT_DMA	6	/* DMA memory read/write error */
#define FDC_STAT_DMAADR	7	/* DMA address out of range
				   (cannot wrap 0xffff -> 0) */
#define FDC_STAT_TRACK	8	/* track # > TRK */
#define FDC_STAT_SEC	9	/* sector # < 1 or > SPT */

extern void fdc_out(BYTE data);
extern BYTE fdc_in(void);

/* external functions that the platform must provide */
extern BYTE read_sec(int drive, int track, int sector, WORD addr);
extern BYTE write_sec(int drive, int track, int sector, WORD addr);
extern void get_fdccmd(BYTE *cmd, WORD addr);

#endif /* !SD_FDC_INC */
