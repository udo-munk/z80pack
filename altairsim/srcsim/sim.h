/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2022 by Udo Munk
 *
 * Configuration for an Altair 8800 system
 *
 * History:
 * 20-OCT-08 first version finished
 * 02-MAR-14 source cleanup and improvements
 * 14-MAR-14 added Tarbell SD FDC and printer port
 * 23-MAR-14 enabled interrupts, 10ms timer added to iosim
 * xx-JUN-14 added default CPU define
 * 19-JUL-14 added typedef for signed 16bit
 * 29-APR-15 added Cromemco DAZZLER to the machine
 * 11-AUG-16 implemented memwrt as function to support ROM
 * 01-DEC-16 implemented memrdr to separate memory from CPU
 * 06-DEC-16 implemented status display and stepping for all machine cycles
 * 12-JAN-17 improved configuration and front panel LED timing
 * 26-FEB-17 added Processor Technology VDM-1 to the machine
 * 27-MAR-17 added SIO's connected to UNIX domain sockets
 * 10-APR-18 trap CPU on unsupported bus data during interrupt
 * 07-MAY-18 added memory configuratione needed by apple monitor
 * 21-AUG-18 improved memory configuration
 * 29-AUG-21 new memory configuration sections
 */

/*
 *	The following defines may be activated, commented or modified
 *	by user for her/his own purpose.
 */
#define DEF_CPU I8080	/* default CPU (Z80 or I8080) */
#define CPU_SPEED 2	/* default CPU speed */
#define Z80_UNDOC	/* compile undocumented Z80 instructions */
/*#define WANT_FASTB*/	/* much faster but not accurate Z80 block instr. */
#define CORE_LOG	/* use LOG() logging in core simulator */

/*#define WANT_ICE*/	/* attach ICE to headless machine */
/*#define WANT_TIM*/	/* don't count t-states */
/*#define HISIZE  1000*//* no history */
/*#define SBSIZE  10*/	/* no breakpoints */

#define HAS_DAZZLER	/* has simulated I/O for Cromemco Dazzler */
#define HAS_DISKS	/* uses disk images */
#define HAS_CONFIG	/* has configuration files somewhere */
#define HAS_BANKED_ROM	/* emulates tarbell banked bootstrap ROM */

#define NUMNSOC 0	/* number of TCP/IP sockets for SIO connections */
#define NUMUSOC 2	/* number of UNIX sockets for SIO connections */

/*
 *	The following defines may be modified and activated by
 *	user, to print her/his copyright for a simulated system,
 *	which contains the Z80/8080 CPU emulations as a part.
 */

#define USR_COM	"Altair 8800 Simulation"
#define USR_REL	"1.19"
#define USR_CPR	"Copyright (C) 2008-2022 by Udo Munk"

#include "simcore.h"
