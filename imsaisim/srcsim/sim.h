/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2024 Udo Munk
 * Copyright (C) 2018-2021 David McNaughton
 *
 * Configuration for an IMSAI 8080 system
 *
 * History:
 * 20-OCT-2008 first version finished
 * 02-MAR-2014 source cleanup and improvements
 * 23-MAR-2014 enabled interrupts, 10ms timer added to iosim
 * xx-JUN-2014 added default CPU define
 * 19-JUL-2014 added typedef for signed 16-bit
 * 09-MAY-2015 added Cromemco DAZZLER to the machine
 * 30-AUG-2016 implemented memwrt as function to support ROM
 * 01-DEC-2016 implemented memrdr to separate memory from CPU
 * 06-DEC-2016 implemented status display and stepping for all machine cycles
 * 12-JAN-2017 improved configuration and front panel LED timing, VIO emulation
 * 10-APR-2018 trap CPU on unsupported bus data during interrupt
 * 14-JUL-2018 integrate webfrontend
 * 12-JUL-2019 implemented second SIO
 * 14-AUG-2020 allow building machine without frontpanel
 * 07-AUG-2021 add APU emulation
 * 29-AUG-2021 new memory configuration sections
 * 09-MAY-2024 added more defines for conditional compiling components
 * 15-MAY-2024 make disk manager standard
 */

#ifndef SIM_INC
#define SIM_INC

/*
 *	The following defines may be activated, commented or modified
 *	by user for her/his own purpose.
 */
#define DEF_CPU I8080	/* default CPU (Z80 or I8080) */
/*#define AMD8080*/	/* AMD 8080 instead of Intel 8080 */
#define CPU_SPEED 2	/* default CPU speed */
#define UNDOC_INST	/* compile undocumented instructions */
#define UNDOC_FLAGS	/* compile undocumented flags */
/*#define FAST_INSTR*/	/* faster instructions, but less debuggable */
/*#define WANT_FASTB*/	/* much faster but not accurate Z80 block instr. */

/*#define WANT_ICE*/	/* attach ICE to headless machine */
/*#define WANT_TIM*/	/* don't count t-states */
/*#define HISIZE  1000*//* no history */
/*#define SBSIZE  10*/	/* no breakpoints */

#define UNIX_TERMINAL	/* uses a UNIX terminal emulation */
#define HAS_DAZZLER	/* has simulated I/O for Cromemeco Dazzler */
/*#define HAS_CYCLOPS*/	/* has simulated I/O for Cromemeco 88 CCC/ACC Cyclops Camera */

#define HAS_DISKS	/* uses disk images */
#define HAS_CONFIG	/* has configuration files somewhere */
#define HAS_BANKED_ROM	/* emulate IMSAI MPU-B banked ROM & RAM */

#define HAS_NETSERVER		/* uses civet webserver to present a web based frontend */
#define NS_DEF_PORT 8080	/* default port number for civet webserver */
#define HAS_MODEM		/* has simulated 'AT' style modem over TCP/IP (telnet) */
#define HAS_APU			/* has simulated AM9511 floating point maths coprocessor */
#define HAS_HAL			/* implements a hardware abstraction layer (HAL) for SIO ports */

#define IMSAISIM
#define MACHINE "imsai"
#define DOCUMENT_ROOT "../webfrontend/www/" MACHINE

#define NUMNSOC 0	/* number of TCP/IP sockets for SIO connections */
#define NUMUSOC 1	/* number of UNIX sockets for SIO connections */

extern void sleep_ms(int);
#define SLEEP_MS(t)	sleep_ms(t)

/*
 *	The following defines may be modified and activated by
 *	user, to print her/his copyright for a simulated system,
 *	which contains the Z80/8080 CPU emulations as a part.
 */

#define USR_COM	"IMSAI 8080 Simulation"
#define USR_REL	"1.19"
#define USR_CPR	"Copyright (C) 2008-2024 by Udo Munk & " \
		"2018-2021 by David McNaughton"

#include "simcore.h"

#endif
