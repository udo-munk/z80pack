/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2022 Udo Munk
 * Copyright (C) 2018-2021 David McNaughton
 *
 * Configuration for an IMSAI 8080 system
 *
 * History:
 * 20-OCT-08 first version finished
 * 02-MAR-14 source cleanup and improvements
 * 23-MAR-14 enabled interrupts, 10ms timer added to iosim
 * xx-JUN-14 added default CPU define
 * 19-JUL-14 added typedef for signed 16-bit
 * 09-MAY-15 added Cromemco DAZZLER to the machine
 * 30-AUG-16 implemented memwrt as function to support ROM
 * 01-DEC-16 implemented memrdr to separate memory from CPU
 * 06-DEC-16 implemented status display and stepping for all machine cycles
 * 12-JAN-17 improved configuration and front panel LED timing, VIO emulation
 * 10-APR-18 trap CPU on unsupported bus data during interrupt
 * 14-JUL-18 integrate webfrontend
 * 12-JUL-19 implemented second SIO
 * 14-AUG-20 allow building machine without frontpanel
 * 07-AUG-21 add APU emulation
 * 29-AUG-21 new memory configuration sections
 */

/*
 *	The following defines may be activated, commented or modified
 *	by user for her/his own purpose.
 */
#define DEF_CPU I8080	/* default CPU (Z80 or I8080) */
#define CPU_SPEED 2	/* default CPU speed */
#define UNDOC_INST	/* compile undocumented instructions */
/*#define WANT_FASTB*/	/* much faster but not accurate Z80 block instr. */
#define CORE_LOG	/* use LOG() logging in core simulator */

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

/*#define HAS_DISKMANAGER*/	/* uses file based disk map for disks[] */
/*#define HAS_NETSERVER*/	/* uses civet webserver to present a web based frontend */
#define HAS_MODEM		/* has simulated 'AT' style modem over TCP/IP (telnet) */
#define HAS_APU			/* has simulated AM9511 floating point maths coprocessor */
#define HAS_HAL			/* implements a hardware abstraction layer (HAL) for SIO ports */

#define IMSAISIM
#define MACHINE "imsai"
#define DOCUMENT_ROOT "../webfrontend/www/" MACHINE

#define NUMNSOC 0	/* number of TCP/IP sockets for SIO connections */
#define NUMUSOC 1	/* number of UNIX sockets for SIO connections */

extern void do_sleep_ms(int);
#define SLEEP_MS(t)	do_sleep_ms(t)

/*
 *	The following defines may be modified and activated by
 *	user, to print her/his copyright for a simulated system,
 *	which contains the Z80/8080 CPU emulations as a part.
 */

#define USR_COM	"IMSAI 8080 Simulation"
#define USR_REL	"1.19"
#define USR_CPR	"\nCopyright (C) 2008-2022 by Udo Munk & " \
		"2018-2021 by David McNaughton"

#include "simcore.h"
