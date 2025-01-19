/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2021 by Udo Munk
 */

#ifndef SIMIO_INC
#define SIMIO_INC

#include "sim.h"
#include "simdefs.h"
#include "unix_network.h"

#define IO_DATA_UNUSED	0xff	/* data returned on unused ports */

extern unix_connector_t ucons[NUMUSOC];

extern in_func_t *const port_in[256];
extern out_func_t *const port_out[256];

extern void init_io(void);
extern void exit_io(void);
extern void reset_io(void);

#endif /* !SIMIO_INC */
