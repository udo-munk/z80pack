/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 Udo Munk
 * Copyright (C) 2021 David McNaughton
 * Copyright (C) 2022-2024 Thomas Eberhardt
 */

#ifndef SIMGLB_INC
#define SIMGLB_INC

/*
 *	Declaration of variables in simglb.c
 */

#include "sim.h"
#include "simdefs.h"

extern int	cpu;

#if !defined(ALT_I8080) && !defined(ALT_Z80)
extern BYTE	A, B, C, D, E, H, L;
extern int	F;
#ifndef EXCLUDE_Z80
extern WORD	IX, IY;
extern BYTE	A_, B_, C_, D_, E_, H_, L_, I, R, R_;
extern int	F_;
#endif
extern WORD	PC, SP;
extern BYTE	IFF;
#else
#include "altregs.h"
#endif
extern Tstates_t T;
extern uint64_t cpu_time;

#ifdef BUS_8080
extern BYTE	cpu_bus;
extern int	m1_step;
#endif

extern BYTE	io_port, io_data;
extern int	busy_loop_cnt;

extern BYTE	cpu_state;
extern int	cpu_error;
#ifndef EXCLUDE_Z80
extern int	int_nmi, int_mode;
#endif
extern int	int_int, int_data, int_protection;
extern BYTE	bus_request;
extern BusDMA_t bus_mode;
extern Tstates_t (*dma_bus_master)(BYTE);
extern int	tmax, cpu_needed;

#ifdef FRONTPANEL
extern uint64_t fp_clock;
extern float	fp_fps;
extern WORD 	fp_led_address;
extern BYTE 	fp_led_data;
extern WORD 	address_switch;
extern BYTE 	fp_led_output;
#endif
#ifdef SIMPLEPANEL
extern WORD 	fp_led_address;
extern BYTE 	fp_led_data;
extern BYTE 	fp_led_output;
#endif

extern int	s_flag, l_flag, m_flag, x_flag, i_flag, f_flag,
		u_flag, r_flag, c_flag;
#ifdef HAS_CONFIG
extern int	M_flag;
#endif
#ifdef HAS_BANKED_ROM
extern int	R_flag;
#endif
#ifdef FRONTPANEL
extern int	F_flag;
#endif
#ifdef HAS_NETSERVER
extern int	n_flag;
#endif

extern char	xfn[MAX_LFN];
#ifdef HAS_DISKS
extern char	*diskdir, diskd[MAX_LFN];
#endif
#ifdef CONFDIR
extern char	confdir[MAX_LFN];
#endif
#ifdef HAS_CONFIG
extern char	conffn[MAX_LFN];
extern char	rompath[MAX_LFN];
#endif

#if !defined(ALT_I8080) || !defined(ALT_Z80)
extern const char parity[256];
#endif

#endif /* !SIMGLB_INC */
