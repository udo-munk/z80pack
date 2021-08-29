/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2021 Udo Munk
 * Copyright (C) 2021 David McNaughton
 *
 * This module reads the system configuration file and sets
 * global variables, so that the system can be configured.
 *
 * History:
 * 20-OCT-08 first version finished
 * 20-MAR-14 ignore carriage return too, necessary for the Windows port
 * 19-JUN-14 added config parameter for droping nulls after CR/LF
 * 09-OCT-14 modified to support 2 SIO's
 * 09-MAY-16 added path for config file
 * 29-AUG-16 ROM and boot switch configuration for Altair emulation added
 * 20-DEC-16 configuration moved local, will be different for each system
 * 30-DEC-16 made RAM size configurable, memory map > E000 is fixed
 * 04-JAN-17 front panel framerate configurable
 * 12-JAN-17 VIO color configurable
 * 26-JAN-17 initial window size of the front panel configurable
 * 21-FEB-17 VIO monitor scanlines configurable
 * 23-FEB-17 added configuration options for VDM
 * 24-MAR-17 added configuration for SIO 0
 * 14-JUN-17 added config for Tarbell boot ROM
 * 07-MAY-18 added memory configuratione needed by apple monitor
 * 03-JUL-18 added baud rate to terminal 2SIO
 * 17-JUL-18 use logging
 * 21-AUG-18 improved memory configuration
 * 24-NOV-19 configurable baud rate for second 2SIO channel
 * 22-JAN-21 added option for config file
 * 31-JUL-21 allow building machine without frontpanel
 * 29-AUG-21 new memory configuration sections
 */

extern void config(void);

extern int  fp_size;
extern BYTE fp_port;
