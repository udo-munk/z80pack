/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2021 by Udo Munk
 */

#ifndef SIMIO_INC
#define SIMIO_INC

#include "sim.h"
#include "unix_network.h"

#define IO_DATA_UNUSED	0xff	/* data returned on unused ports */

extern struct unix_connectors ucons[NUMUSOC];

extern BYTE (*const port_in[256])(void);
extern void (*const port_out[256])(BYTE data);

extern void init_io(void);
extern void exit_io(void);
extern void reset_io(void);

#endif /* !SIMIO_INC */
