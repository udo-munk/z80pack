/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 *
 * History:
 * 28-SEP-87 Development on TARGON/35 with AT&T Unix System V.3
 * 11-JAN-89 Release 1.1
 * 08-FEB-89 Release 1.2
 * 13-MAR-89 Release 1.3
 * 09-FEB-90 Release 1.4  Ported to TARGON/31 M10/30
 * 20-DEC-90 Release 1.5  Ported to COHERENT 3.0
 * 10-JUN-92 Release 1.6  long casting problem solved with COHERENT 3.2
 *			  and some optimisation
 * 25-JUN-92 Release 1.7  comments in english and ported to COHERENT 4.0
 * 02-OCT-06 Release 1.8  modified to compile on modern POSIX OS's
 * 18-NOV-06 Release 1.9  modified to work with CP/M sources
 * 08-DEC-06 Release 1.10 modified MMU for working with CP/NET
 * 17-DEC-06 Release 1.11 TCP/IP sockets for CP/NET
 * 25-DEC-06 Release 1.12 CPU speed option
 * 19-FEB-07 Release 1.13 various improvements
 * 06-OCT-07 Release 1.14 bug fixes and improvements
 * 06-AUG-08 Release 1.15 many improvements and Windows support via Cygwin
 * 25-AUG-08 Release 1.16 console status I/O loop detection and line discipline
 * 20-OCT-08 Release 1.17 frontpanel integrated and Altair/IMSAI emulations
 * 24-JAN-14 Release 1.18 bug fixes and improvements
 * 02-MAR-14 Release 1.19 source cleanup and improvements
 * 14-MAR-14 Release 1.20 added Tarbell SD FDC and printer port to Altair
 * 29-MAR-14 Release 1.21 many improvements
 * 29-MAY-14 Release 1.22 improved networking and bugfixes
 * 04-JUN-14 Release 1.23 added 8080 emulation
 * 06-SEP-14 Release 1.24 bugfixes and improvements
 * 18-FEB-15 Release 1.25 bugfixes, improvements, added Cromemco Z-1
 * 18-APR-15 Release 1.26 bugfixes and improvements
 * 18-JAN-16 Release 1.27 bugfixes and improvements
 * 05-MAY-16 Release 1.28 improved usability
 * 20-NOV-16 Release 1.29 bugfixes and improvements
 * 15-DEC-16 Release 1.30 improved memory management, machine cycle correct CPUs
 * 28-DEC-16 Release 1.31 improved memory management, reimplemented MMUs
 * 12-JAN-17 Release 1.32 improved configurations, front panel, added IMSAI VIO
 * 07-FEB-17 Release 1.33 bugfixes, improvements, better front panels
 * 16-MAR-17 Release 1.34 improvements, added ProcTec VDM-1
 * 03-AUG-17 Release 1.35 added UNIX sockets, bugfixes, improvements
 * 21-DEC-17 Release 1.36 bugfixes and improvements
 * 06-JAN-21 Release 1.37 bugfixes and improvements
 */

/*
 *	Declaration of variables in simglb.c
 */

extern int	cpu;

extern BYTE	A, B, C, D, E, H, L, A_, B_, C_, D_, E_, H_, L_, I, IFF;
extern WORD	PC, SP, IX, IY;
extern int	F, F_;
extern long	R;
extern BYTE	io_port, io_data;

#ifdef BUS_8080
extern BYTE	cpu_bus;
extern int	m1_step;
#endif

extern BYTE	cpu_state, bus_request;
extern int	int_data;

extern int	s_flag, l_flag, m_flag, x_flag, break_flag, i_flag, f_flag,
		u_flag, r_flag, c_flag,
		cpu_error, int_nmi, int_int, int_mode, parity[], sb_next,
		int_protection;

extern int	tmax, cpu_needed;
extern int	busy_loop_cnt[];

extern char	xfn[];
extern char	*diskdir, diskd[];
extern char	confdir[], conffn[];

#ifdef HISIZE
extern struct	history his[];
extern int	h_next, h_flag;
#endif

#ifdef SBSIZE
extern struct	softbreak soft[];
#endif

extern long	t_states;
extern int	t_flag;
extern WORD	t_start, t_end;

#ifdef FRONTPANEL
extern unsigned long long fp_clock;
extern float	fp_fps;
extern WORD 	fp_led_address;
extern BYTE 	fp_led_data;
extern WORD 	address_switch;
extern BYTE 	fp_led_output;
#endif
