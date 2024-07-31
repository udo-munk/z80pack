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
 * 14-JUN-2017 added config for Tarbell boot ROM
 * 07-MAY-2018 added memory configuration needed by apple monitor
 * 03-JUL-2018 added baud rate to terminal 2SIO
 * 17-JUL-2018 use logging
 * 21-AUG-2018 improved memory configuration
 * 24-NOV-2019 configurable baud rate for second 2SIO channel
 * 22-JAN-2021 added option for config file
 * 31-JUL-2021 allow building machine without frontpanel
 * 29-AUG-2021 new memory configuration sections
 */

#ifndef SIMCFG_INC
#define SIMCFG_INC

#include "sim.h"
#include "simdefs.h"

extern int  fp_size;
extern BYTE fp_port;

extern void config(void);

#endif /* !SIMCFG_H */
