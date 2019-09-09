/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2019 by Udo Munk
 *
 * Emulation of a Cromemco 4FDC/16FDC S100 board
 *
 * History:
 * 20-DEC-2014 first version
 * 28-DEC-2014 second version with 16FDC, CP/M 2.2 boots
 * 01-JAN-2015 fixed 16FDC, machine now also boots CDOS 2.58 from 8" and 5.25"
 * 01-JAN-2015 fixed frontpanel switch settings, added boot flag to fp switch
 * 12-JAN-2015 implemented dummy write track, so that programs won't hang
 * 22-JAN-2015 fixed buggy ID field, fake index pulses for 300/360 RPM
 * 26-JAN-2015 implemented write track to format SS/SD disks properly
 * 02-FEB-2015 implemented DS/DD disk formats
 * 05-FEB-2015 implemented DS/SD disk formats
 * 06-FEB-2015 implemented write track for all formats
 * 12-FEB-2015 implemented motor control, so that a 16FDC is recogniced by CDOS
 * 20-FEB-2015 bug fixes for 1.25 release
 * 08-MAR-2016 support user path for disk images
 * 13-MAY-2016 find disk images at -d <path>, ./disks and DISKDIR
 * 22-JUL-2016 added support for read only disks
 * 22-JUN-2017 added reset function
 * 26-JUL-2017 fixed buggy index pulse implementation
 * 15-AUG-2017 and more fixes for index pulse
 * 22-AUG-2017 provide write protect and track 0 bits for all commands
 * 23-APR-2018 cleanup
 * 20-MAY-2018 improved reset
 * 15-JUL-2018 use logging
 * 09-SEP-2019 added disk format without SD track 0 provided by Alan Cox
 */

/*
 * disk definitions 5.25"/8" drives, single/double density,
 * single/double sided, write protected
 */
enum Disk_type { SMALL, LARGE, UNKNOWN };
enum Disk_density { SINGLE, DOUBLE };
enum Disk_sides { ONE, TWO };
enum Disk_mode { READWRITE, READONLY };

typedef struct {
	char *fn;			/* filename of disk image */
	enum Disk_type disk_t;		/* drive type 5.25" or 8" */
	enum Disk_density disk_d;	/* disk density, single or double */
	enum Disk_sides disk_s;		/* drive sides, 1 or 2 */
	int tracks;			/* # of tracks */
	int sectors;			/* # sectors on tracks > 0 side 0 */
	int sec0;			/* # sectors on track 0, side 0 */
	enum Disk_mode disk_m;		/* R/W or R/O mode */
	enum Disk_density disk_d0;	/* Density of track 0 */
} Diskdef;

extern BYTE fdc_flags;
extern int index_pulse;
extern int motoron, motortimer;
extern enum Disk_type dtype;

extern BYTE cromemco_fdc_status_in(void);
extern void cromemco_fdc_cmd_out(BYTE);

extern BYTE cromemco_fdc_track_in(void);
extern void cromemco_fdc_track_out(BYTE);

extern BYTE cromemco_fdc_sector_in(void);
extern void cromemco_fdc_sector_out(BYTE);

extern BYTE cromemco_fdc_data_in(void);
extern void cromemco_fdc_data_out(BYTE);

extern BYTE cromemco_fdc_diskflags_in(void);
extern void cromemco_fdc_diskctl_out(BYTE);

extern BYTE cromemco_fdc_aux_in(void);
extern void cromemco_fdc_aux_out(BYTE);

extern void cromemco_fdc_reset(void);
