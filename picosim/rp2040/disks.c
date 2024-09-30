/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk & Thomas Eberhardt
 *
 * This module implements the disks drives and low level
 * access functions for MicroSD, needed by the FDC.
 *
 * History:
 * 27-MAY-2024 implemented load file
 * 28-MAY-2024 implemented sector I/O to disk images
 * 03-JUN-2024 added directory list for code files and disk images
 * 29-JUN-2024 split of from memsim.c and picosim.c
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simmem.h"

#include "gpio.h"
#include "ff.h"
#include "f_util.h"
#include "hw_config.h"

#include "sd-fdc.h"
#include "disks.h"
#include "rgbled.h"

FIL sd_file;	/* at any time we have only one file open */
FRESULT sd_res;	/* result code from FatFS */
char disks[NUMDISK][DISKLEN]; /* path name for 4 disk images /DISKS80/filename.DSK */

static FATFS fs; /* FatFs on MicroSD */

/* buffer for disk/memory transfers */
static unsigned char dsk_buf[SEC_SZ];

/* global variables for access to MicroSD card */

/* Configuration of hardware SPI object */
static spi_t spi = {
	.hw_inst = SD_SPI_PORT,
	.sck_gpio = SD_SPI_CLK,
	.mosi_gpio = SD_SPI_SI,
	.miso_gpio = SD_SPI_SO,
#if PICO_RP2040
	//.baud_rate = 125 * 1000 * 1000 / 8, /* 15.625 MHz */
	//.baud_rate = 125 * 1000 * 1000 / 6, /* 20.833333 MHz */
	.baud_rate = 125 * 1000 * 1000 / 4, /* 31.25 MHz */
#endif
#if PICO_RP2350
	//.baud_rate = 150 * 1000 * 1000 / 8,	// 18.75 MHz
	//.baud_rate = 150 * 1000 * 1000 / 6,	// 25.00 MHz
	.baud_rate = 150 * 1000 * 1000 / 4,	// 37.50 MHz
#endif
	.spi_mode = 3,
	.set_drive_strength = true,
	.mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_8MA,
	.sck_gpio_drive_strength = GPIO_DRIVE_STRENGTH_8MA
};

/* SPI Interface */
static sd_spi_if_t spi_if = {
	.spi = &spi,		/* Pointer to the SPI driving this card */
	.ss_gpio = SD_SPI_CS	/* The SPI slave select GPIO for this SD card */
};

/* Configuration of the SD Card socket object */
static sd_card_t sd_card = {
	.type = SD_IF_SPI,
	.spi_if_p = &spi_if /* Pointer to the SPI interface driving this card */
};

/* Callbacks used by the SD library */

size_t sd_get_num(void)
{
	return 1;
}

sd_card_t *sd_get_by_num(size_t num)
{
	if (num == 0) {
		return &sd_card;
	} else {
		return NULL;
	}
}

void init_disks(void)
{
	/* try to mount SD card */
	sd_res = f_mount(&fs, "", 1);
	if (sd_res != FR_OK)
		panic("f_mount error: %s (%d)\n", FRESULT_str(sd_res), sd_res);
}

void exit_disks(void)
{
	/* unmount SD card */
	f_unmount("");
}

/*
 * list files with pattern 'ext' in directory 'dir'
 */
void list_files(const char *dir, const char *ext)
{
	DIR dp;
	FILINFO fno;
	FRESULT res;
	register int i = 0;

	res = f_findfirst(&dp, &fno, dir, ext);
	if (res == FR_OK) {
		while (1) {
			printf("%s\t", fno.fname);
			if (strlen(fno.fname) < 8)
				putchar('\t');
			i++;
			if (i > 4) {
				putchar('\n');
				i = 0;
			}
			res = f_findnext(&dp, &fno);
			if (!strlen(fno.fname)) {
				if (i > 0)
					putchar('\n');
				break;
			}
		}
	}
}

/*
 * load a file 'name' into memory
 */
void load_file(const char *name)
{
	int i = 0;
	register unsigned int j;
	unsigned int br;
	char SFN[25];

	strcpy(SFN, "/CODE80/");
	strcat(SFN, name);
	strcat(SFN, ".BIN");

	/* try to open file */
	sd_res = f_open(&sd_file, SFN, FA_READ);
	if (sd_res != FR_OK) {
		puts("File not found");
		return;
	}

	/* read file into memory */
	while ((sd_res = f_read(&sd_file, &dsk_buf[0], SEC_SZ, &br)) == FR_OK) {
		for (j = 0; j < br; j++)
			dma_write(i + j, dsk_buf[j]);
		if (br < SEC_SZ)	/* last record reached */
			break;
		i += SEC_SZ;
	}
	if (sd_res != FR_OK)
		printf("f_read error: %s (%d)\n", FRESULT_str(sd_res), sd_res);
	else
		printf("loaded file \"%s\" (%d bytes)\n", SFN, i + br);

	f_close(&sd_file);
}

/*
 * check that all disks refer to existing files
 */
void check_disks(void)
{
	int i, n = 0;

	for (i = 0; i < NUMDISK; i++) {
		if (disks[i][0]) {
			/* try to open file */
			sd_res = f_open(&sd_file, disks[i], FA_READ);
			if (sd_res != FR_OK) {
				printf("Disk image \"%s\" no longer exists.\n",
				       disks[i]);
				disks[i][0] = '\0';
				n++;
			} else
				f_close(&sd_file);
		}
	}
	if (n > 0)
		putchar('\n');
}

/*
 * mount a disk image 'name' on disk 'drive'
 */
void mount_disk(int drive, const char *name)
{
	char SFN[DISKLEN];
	int i;

	strcpy(SFN, "/DISKS80/");
	strcat(SFN, name);
	strcat(SFN, ".DSK");

	for (i = 0; i < NUMDISK; i++) {
		if (i != drive && strcmp(disks[i], SFN) == 0) {
			puts("Disk already mounted\n");
			return;
		}
	}

	/* try to open file */
	sd_res = f_open(&sd_file, SFN, FA_READ);
	if (sd_res != FR_OK) {
		puts("File not found\n");
		return;
	}

	f_close(&sd_file);
	strcpy(disks[drive], SFN);
	putchar('\n');
}

/*
 * prepare I/O for sector read and write routines
 */
static BYTE prep_io(int drive, int track, int sector, WORD addr)
{
	FSIZE_t pos;

	/* check if drive in range */
	if ((drive < 0) || (drive > 3))
		return FDC_STAT_DISK;

	/* check if track and sector in range */
	if (track > TRK)
		return FDC_STAT_TRACK;
	if ((sector < 1) || (sector > SPT))
		return FDC_STAT_SEC;

	/* check if DMA address in range */
	if (addr > 0xff7f)
		return FDC_STAT_DMAADR;

	/* check if disk in drive */
	if (!strlen(disks[drive])) {
		return FDC_STAT_NODISK;
	}

	/* open file with the disk image */
	sd_res = f_open(&sd_file, disks[drive], FA_READ | FA_WRITE);
	if (sd_res != FR_OK)
		return FDC_STAT_NODISK;

	/* seek to track/sector */
	pos = (((FSIZE_t) track * (FSIZE_t) SPT) + sector - 1) * SEC_SZ;
	if (f_lseek(&sd_file, pos) != FR_OK) {
		f_close(&sd_file);
		return FDC_STAT_SEEK;
	}
	return FDC_STAT_OK;
}

/*
 * read from drive a sector on track into memory @ addr
 */
BYTE read_sec(int drive, int track, int sector, WORD addr)
{
	BYTE stat;
	unsigned int br;
	register int i;

	/* prepare for sector read */
	if ((stat = prep_io(drive, track, sector, addr)) != FDC_STAT_OK)
		return stat;

	put_pixel(0x440000); /* LED green */

	/* read sector into memory */
	sd_res = f_read(&sd_file, &dsk_buf[0], SEC_SZ, &br);
	if (sd_res == FR_OK) {
		if (br < SEC_SZ) {	/* UH OH */
			f_close(&sd_file);
			stat = FDC_STAT_READ;
		} else {
			f_close(&sd_file);
			for (i = 0; i < SEC_SZ; i++)
				dma_write(addr + i, dsk_buf[i]);
			stat = FDC_STAT_OK;
		}
	} else {
		f_close(&sd_file);
		stat = FDC_STAT_READ;
	}

	sleep_us(300);
	put_pixel(0x000000); /* LED off */
	return stat;
}

/*
 * write to drive a sector on track from memory @ addr
 */
BYTE write_sec(int drive, int track, int sector, WORD addr)
{
	BYTE stat;
	unsigned int br;
	register int i;

	/* prepare for sector write */
	if ((stat = prep_io(drive, track, sector, addr)) != FDC_STAT_OK)
		return stat;

	put_pixel(0x004400); /* LED red */

	/* write sector to disk image */
	for (i = 0; i < SEC_SZ; i++)
		dsk_buf[i] = dma_read(addr + i);
	sd_res = f_write(&sd_file, &dsk_buf[0], SEC_SZ, &br);
	if (sd_res == FR_OK) {
		if (br < SEC_SZ) {	/* UH OH */
			f_close(&sd_file);
			stat = FDC_STAT_WRITE;
		} else {
			f_close(&sd_file);
			stat = FDC_STAT_OK;
		}
	} else {
		f_close(&sd_file);
		stat = FDC_STAT_WRITE;
	}

	sleep_us(300);
	put_pixel(0x000000); /* LED off */
	return stat;
}

/*
 * get FDC command from CPU memory
 */
void get_fdccmd(BYTE *cmd, WORD addr)
{
	register int i;

	for (i = 0; i < 4; i++)
		cmd[i] = dma_read(addr + i);
}
