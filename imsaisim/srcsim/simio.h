/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2024 by Udo Munk
 * Copyright (C) 2021 David McNaughton
 */

#ifndef SIMIO_INC
#define SIMIO_INC

#include "sim.h"
#include "simdefs.h"

#include "unix_network.h"

#define IO_DATA_UNUSED	0xff	/* data returned on unused ports */

extern void lpt_reset(void);

extern struct unix_connectors ucons[NUMUSOC]; /* socket connections for SIO's */

extern BYTE (*const port_in[256])(void);
extern void (*const port_out[256])(BYTE data);

extern void init_io(void);
extern void exit_io(void);
extern void reset_io(void);

#endif /* !SIMIO_INC */
