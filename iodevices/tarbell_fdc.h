/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2019 by Udo Munk
 *
 * Emulation of a Tarbell SD 1011D S100 board
 *
 * History:
 * 13-MAR-2014 first fully working version
 * 15-MAR-2014 some improvements for CP/M 1.3 & 1.4
 * 17-MAR-2014 close(fd) was missing in write sector lseek error case
 *    AUG-2014 some improvements
 * 22-JAN-2015 fixed buggy ID field
 * 11-FEB-2015 implemented write track
 * 08-MAR-2016 support user path for disk images
 * 13-MAY-2016 find disk images at -d <path>, ./disks and DISKDIR
 * 22-JUL-2016 added support for read only disks
 * 13-JUN-2017 added bootstrap ROM and reset function
 * 23-APR-2018 cleanup
 * 01-JUL-2018 check disk images for the correct size
 * 15-JUL-2018 use logging
 * 23-SEP-2019 bug fixes and improvements by Mike Douglas
 * 24-SEP-2019 restore and seek also affect step direction
 */

#ifndef TARBELL_FDC_INC
#define TARBELL_FDC_INC

#include "sim.h"
#include "simdefs.h"

extern BYTE tarbell_rom[32];
extern bool tarbell_rom_enabled, tarbell_rom_active;

extern void dsk_path(void);

extern BYTE tarbell_stat_in(void), tarbell_track_in(void);
extern BYTE tarbell_sec_in(void), tarbell_data_in(void);
extern BYTE tarbell_wait_in(void);
extern void tarbell_cmd_out(BYTE data), tarbell_track_out(BYTE data);
extern void tarbell_sec_out(BYTE data), tarbell_data_out(BYTE data);
extern void tarbell_ext_out(BYTE data);
extern void tarbell_reset(void);

#endif /* !TARBELL_FDC_INC */
