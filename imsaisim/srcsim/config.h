/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2021 by Udo Munk
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
 * 18-JUL-18 use logging
 * 12-JUL-19 implemented second SIO
 * 17-SEP-19 more consistent SIO naming
 * 07-OCT-19 implemented baud rate for modem device
 * 14-AUG-20 allow building machine without frontpanel
 * 22-JAN-21 added option for config file
 * 14-JUL-21 added all options for SIO 2B
 * 16-JUL-21 added all options for SIO 1B
 * 20-JUL-21 log banked memory
 */

extern void config(void);

extern int ram_size;
extern int fp_size;
