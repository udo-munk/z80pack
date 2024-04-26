/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 Udo Munk
 * Copyright (C) 2021 David McNaughton
 * Copyright (C) 2022-2024 Thomas Eberhardt
 */

/*
 *	Declaration of variables in simglb.c
 */

typedef unsigned long long Tstates_t;	/* uint64 for counting T-states */
typedef enum { BUS_DMA_NONE, BUS_DMA_BYTE,
	       BUS_DMA_BURST, BUS_DMA_CONTINUOUS } BusDMA_t;

void start_bus_request(BusDMA_t mode, Tstates_t (*bus_master)(BYTE bus_ack));
void end_bus_request(void);

extern int	cpu;

extern BYTE	A, B, C, D, E, H, L;
extern int	F;
#ifndef EXCLUDE_Z80
extern WORD	IX, IY;
extern BYTE	A_, B_, C_, D_, E_, H_, L_, I, R_;
extern int	F_;
#endif
extern WORD	PC, SP;
extern BYTE	IFF;
extern long	R;
extern Tstates_t T;
extern BYTE	io_port, io_data;

#ifdef BUS_8080
extern BYTE	cpu_bus;
extern int	m1_step;
#endif

extern BYTE	cpu_state, bus_request;
extern BusDMA_t bus_mode;
extern Tstates_t (*dma_bus_master)(BYTE bus_ack);
extern int	int_data;

extern int	s_flag, l_flag, m_flag, x_flag, break_flag, i_flag, f_flag,
		u_flag, r_flag, c_flag, M_flag, R_flag,
		cpu_error, int_int, parity[], sb_next, int_protection;
#ifndef EXCLUDE_Z80
extern int	int_nmi, int_mode;
#endif

extern int	tmax, cpu_needed;
extern int	busy_loop_cnt[];

extern char	xfn[];
extern char	*diskdir, diskd[];
extern char	confdir[], conffn[];
extern char	rompath[];

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

extern void (*ice_before_go)(void);
extern void (*ice_after_go)(void);
extern void (*ice_cust_cmd)(char *, WORD *);
extern void (*ice_cust_help)(void);
