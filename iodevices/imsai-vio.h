/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2017-2019 by Udo Munk
 * Copyright (C) 2018 David McNaughton
 *
 * Emulation of an IMSAI VIO S100 board
 *
 * History:
 * 10-JAN-2017 80x24 display output tested and working
 * 11-JAN-2017 implemented keyboard input for the X11 key events
 * 12-JAN-2017 all resolutions in all video modes tested and working
 * 04-FEB-2017 added function to terminate thread and close window
 * 21-FEB-2017 added scanlines to monitor
 * 20-APR-2018 avoid thread deadlock on Windows/Cygwin
 * 07-JUL-2018 optimization
 * 12-JUL-2018 use logging
 * 14-JUL-2018 integrate webfrontend
 * 05-NOV-2019 use correct memory access function
 */

extern void imsai_vio_init(void), imsai_vio_off(void);
