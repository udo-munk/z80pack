/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

#ifndef SIMICE_INC
#define SIMICE_INC

#include "sim.h"
#include "simdefs.h"

#ifdef WANT_ICE

#ifdef HISIZE
typedef struct history {	/* structure of a history entry */
	int	h_cpu;		/* CPU type */
	WORD	h_addr;		/* address of execution */
	WORD	h_af;		/* register AF */
	WORD	h_bc;		/* register BC */
	WORD	h_de;		/* register DE */
	WORD	h_hl;		/* register HL */
#ifndef EXCLUDE_Z80
	WORD	h_ix;		/* register IX */
	WORD	h_iy;		/* register IY */
#endif
	WORD	h_sp;		/* register SP */
} history_t;

extern history_t his[HISIZE];
extern int	h_next;
extern bool	h_flag;
#endif

#ifdef SBSIZE
typedef struct softbreak {	/* structure of a breakpoint */
	WORD	sb_addr;	/* address of breakpoint */
	BYTE	sb_oldopc;	/* op-code at address of breakpoint */
	int	sb_passcount;	/* pass counter of breakpoint */
	int	sb_pass;	/* no. of pass to break */
} softbreak_t;

extern softbreak_t soft[SBSIZE];
#endif

#ifdef WANT_TIM
extern Tstates_t t_states_s, t_states_e;
extern bool	t_flag;
extern WORD	t_start, t_end;
#endif

#ifdef WANT_HB
				/* hardware breakpoint access modes */
#define HB_READ		1	/* read memory */
#define HB_WRITE	2	/* write memory */
#define HB_EXEC		4	/* execute (op-code fetch) */

extern bool	hb_flag;
extern int	hb_mode, hb_trig;
extern WORD	hb_addr;
#endif

extern void (*ice_before_go)(void);
extern void (*ice_after_go)(void);
extern void (*ice_cust_cmd)(char *cmd, WORD *wrk_addr);
extern void (*ice_cust_help)(void);

extern void ice_cmd_loop(int go_mode);

#endif /* WANT_ICE */

#endif /* !SIMICE_INC */
