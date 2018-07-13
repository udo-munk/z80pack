/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2018 by Udo Munk
 *
 * Emulation of an IMSAI FIF S100 board
 *
 * History:
 * 18-JAN-2014 first version finished
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
 * 13-JUL-2018 use logging
 */

extern BYTE imsai_fif_in(void);
extern void imsai_fif_out(BYTE);

extern void imsai_fif_reset(void);
