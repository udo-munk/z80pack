/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

#ifndef SIMIO_INC
#define SIMIO_INC

#include "sim.h"
#include "simdefs.h"

#include "unix_network.h"

#define IO_DATA_UNUSED	0x00	/* data returned on unused ports */

extern BYTE int_requests;

extern int lpt_fd;

extern struct net_connectors ncons[NUMNSOC];
extern struct unix_connectors ucons[NUMUSOC];

extern BYTE (*const port_in[256])(void);
extern void (*const port_out[256])(BYTE data);

extern void init_io(void);
extern void exit_io(void);
extern void reset_io(void);

extern void int_request(int irq);
extern void int_cancel(int irq);

#ifdef WANT_ICE
extern void ice_go(void);
extern void ice_break(void);
#endif

#endif /* !SIMIO_INC */
