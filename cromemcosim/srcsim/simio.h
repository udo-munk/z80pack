/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2014-2022 Udo Munk
 */

#ifndef SIMIO_INC
#define SIMIO_INC

#include "sim.h"
#include "unix_network.h"

#define IO_DATA_UNUSED	0xff	/* data returned on unused ports */

extern int lpt1, lpt2;

extern struct net_connectors ncons[NUMNSOC];

extern BYTE (*const port_in[256])(void);
extern void (*const port_out[256])(BYTE data);

extern void init_io(void);
extern void exit_io(void);
extern void reset_io(void);

#ifdef WANT_ICE
extern void ice_go(void);
extern void ice_break(void);
#endif

#endif /* !SIMIO_INC */
