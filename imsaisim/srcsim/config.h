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
 * 20-OCT-2008 first version finished
 * 20-MAR-2014 ignore carriage return too, necessary for the Windows port
 * 19-JUN-2014 added config parameter for dropping nulls after CR/LF
 * 09-OCT-2014 modified to support 2 SIO's
 * 09-MAY-2016 added path for config file
 * 29-AUG-2016 ROM and boot switch configuration for Altair emulation added
 * 20-DEC-2016 configuration moved local, will be different for each system
 * 30-DEC-2016 made RAM size configurable, memory map > E000 is fixed
 * 04-JAN-2017 front panel framerate configurable
 * 12-JAN-2017 VIO color configurable
 * 26-JAN-2017 initial window size of the front panel configurable
 * 21-FEB-2017 VIO monitor scanlines configurable
 * 23-FEB-2017 added configuration options for VDM
 * 24-MAR-2017 added configuration for SIO 0
 * 18-JUL-2018 use logging
 * 12-JUL-2019 implemented second SIO
 * 17-SEP-2019 more consistent SIO naming
 * 07-OCT-2019 implemented baud rate for modem device
 * 14-AUG-2020 allow building machine without frontpanel
 * 22-JAN-2021 added option for config file
 * 14-JUL-2021 added all options for SIO 2B
 * 16-JUL-2021 added all options for SIO 1B
 * 20-JUL-2021 log banked memory
 * 05-AUG-2021 add boot config for machine without frontpanel
 * 29-AUG-2021 new memory configuration sections
 */

extern void config(void);

extern int  fp_size;
extern BYTE fp_port;
